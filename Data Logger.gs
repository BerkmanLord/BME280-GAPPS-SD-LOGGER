// Example Google Scrips code to upload data to Google Sheets from Arduino/ESP8266
// Follow setup instructions found here:
// https://github.com/StorageB/Google-Sheets-Logging
// contact: storageunitb@gmail.com
//
// This example uses the HTTPSRedirect library by Sujay Phadke
// https://github.com/electronicsguy/ESP8266


// Enter Spreadsheet ID here
var SS = SpreadsheetApp.openById('YOURSPREADSHEETID');

// Sheet Name, Chnage Sheet1 to Users in Spread Sheet. Or any other name as you wish
var sheet = SS.getSheetByName("Device Controls");

var str = "";


function doGet(e){



  
 return getControls(sheet); 
  
}


function getControls(sheet){
  var jo = {};
  var dataArray = [];

// collecting data from 2nd Row , 1st column to last row and last column
  var rows = sheet.getRange(2,1,sheet.getLastRow()-1, sheet.getLastColumn()).getValues();
  
  for(var i = 0, l= rows.length; i<l ; i++){
    var dataRow = rows[i];
    var record = {};
    record['id'] = dataRow[0];
    record['state'] = dataRow[1];
    
    dataArray.push(record);
    
  }  
  
  jo.device = dataArray;
  
  var result = JSON.stringify(jo);
  
  return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.JSON);
  
}  



function doPost(e) {

  var parsedData;
  var result = {};
  
  try { 
    parsedData = JSON.parse(e.postData.contents);
  } 
  catch(f){
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }
   
  if (parsedData !== undefined){
    var flag = parsedData.format;
    if (flag === undefined){
      flag = 0;
    }
    
    var sheet = SS.getSheetByName(parsedData.sheet_name); // sheet name to publish data to is specified in Arduino code
    var dataArr = parsedData.values.split(","); // creates an array of the values to publish 
         
    var date_now = Utilities.formatDate(new Date(), SpreadsheetApp.getActiveSpreadsheet().getSpreadsheetTimeZone(), "dd.MM.yyyy"); // gets the current date specific to the sheet Time Zone
    var time_now = Utilities.formatDate(new Date(), SpreadsheetApp.getActiveSpreadsheet().getSpreadsheetTimeZone(), "hh:mm:ss"); // gets the current time specific to the sheet Time Zone
    
    var uploadCounter = dataArr [0]; // value0 from Arduino code
    var temp = dataArr [1]; // value1 from Arduino code
    var hum = dataArr [2]; // value2 from Arduino code
    var psr = dataArr [3]; // buttonstate from Arduino code
    
    
    // read and execute command from the "payload_base" string specified in Arduino code
    switch (parsedData.command) {
      
      case "insert_row":
         
         sheet.insertRows(2); // insert full row directly below header text
         
         //var range = sheet.getRange("A2:D2");              // use this to insert cells just above the existing data instead of inserting an entire row
         //range.insertCells(SpreadsheetApp.Dimension.ROWS); // use this to insert cells just above the existing data instead of inserting an entire row
         
         sheet.getRange('A2').setValue(date_now); // publish current date to cell A2
         sheet.getRange('B2').setValue(time_now); // publish current time to cell B2
         sheet.getRange('C2').setValue(uploadCounter);   // publish value0 from Arduino code to cell C2
         sheet.getRange('D2').setValue(temp);   // publish value1 from Arduino code to cell D2
         sheet.getRange('E2').setValue(hum);   // publish value2 from Arduino code to cell E2
         sheet.getRange('F2').setValue(psr);   // publish buttonState from Arduino code to cell F2
         
         str = "Success"; // string to return back to Arduino serial console
         SpreadsheetApp.flush();
         break;
         
      case "append_row":
         
         var publish_array = new Array(); // create a new array
         
         publish_array [0] = date_now; // add current date to position 0 in publish_array
         publish_array [1] = time_now; // add current time to position 1 in publish_array
         publish_array [2] = uploadCounter;   // add value0 from Arduino code to position 2 in publish_array
         publish_array [3] = temp;   // add value1 from Arduino code to position 3 in publish_array
         publish_array [4] = hum;   // add value2 from Arduino code to position 4 in publish_array
         publish_array [5] = psr;   // add buttonState from Arduino code to position 5 in publish_array
         
         sheet.appendRow(publish_array); // publish data in publish_array after the last row of data in the sheet
         
         str = "Success"; // string to return back to Arduino serial console
         SpreadsheetApp.flush();
         break;     
 
    }
    
    return ContentService.createTextOutput(str);
  } // endif (parsedData !== undefined)
  
  else{
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }