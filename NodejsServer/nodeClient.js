console.log("Node Server Start");

function encryptString(text) {
   const substitutionMap = {
     'a': 'z',
     'b': 'y',
     'c': 'x',
     'd': 'w',
     'e': 'v',
     'f': 'u',
     'g': 't',
     'h': 's',
     'i': 'r',
     'j': 'q',
     'k': 'p',
     'l': 'o',
     'm': 'n',
     'n': 'm',
     'o': 'l',
     'p': 'k',
     'q': 'j',
     'r': 'i',
     's': 'h',
     't': 'g',
     'u': 'f',
     'v': 'e',
     'w': 'd',
     'x': 'c',
     'y': 'b',
     'z': 'a',
     ' ': '_'
   };
 
   let encryptedText = '';
   for (let i = 0; i < text.length; i++) {
     const char = text[i];
     const substitution = substitutionMap[char] || char;
     encryptedText += substitution;
   }
 
   return encryptedText;
}
  
const net = require('net');
const http = require('http');
const express = require('express');
const cors = require('cors');
const { group } = require('console');

// Function to send data to the C server
function testSocketServer(socketAddress, socketPort) {
   return new Promise((resolve, reject) => {
      // Create a new socket connection to the C server
      const client = new net.Socket();
      let connectionStatus = 0;
   
      // Connect to the C server
      client.connect(socketPort, socketAddress, () => {
         // Connection to C server established
         console.log('Connected to C server');

         // Send the data to the C server
         client.write('test');
      });

      // Handle data received from the server
      client.on('data', (data) => {
         console.log('Received data from server:', data.toString());
         connectionStatus = 1;
         client.end();
         resolve(connectionStatus); // Resolve the Promise with 1 (success)
      });
   
      // Handle connection error
      client.on('error', (error) => {
         console.error('Error connecting to C server:', error);
         // Close the socket connection
         client.destroy();
         // Reject the Promise with the error
         reject(error);
      });

      // Handle connection close
      client.on('close', (error) => {
         if (error) {
            console.error('Connection to C server closed due to an error');
            // Handle the error appropriately
            reject(new Error('Connection error'));
         } else {
            console.log('Connection to C server closed');
            // Clean up any resources or perform necessary actions
            resolve(connectionStatus); // Resolve the Promise with 1 (success)
         }
      });

   });
}

// Function to send login data to the C server
function tryLogin(socketAddress, socketPort, username, password) {
   return new Promise((resolve, reject) => {
      // Create a new socket connection to the C server
      const client = new net.Socket();
      let loginStatus = 0;
   
      // Connect to the C server
      client.connect(socketPort, socketAddress, () => {
         // Connection to C server established
         console.log('Connected to C server (login)');

         // Send the data to the C server
         client.write('login\n'+username+'\n'+password);
      });

      // Handle data received from the server
      client.on('data', (data) => {
         console.log('Received data from server:', data.toString());
         if(data.toString() === '1'){
            loginStatus = 1;
         }else{
            loginStatus = 0;
         } 
         client.end();
         resolve(loginStatus); // Resolve the Promise with 1 (success)
      });
   
      // Handle connection error
      client.on('error', (error) => {
         console.error('Error connecting to C server:', error);
         // Close the socket connection
         client.destroy();
         // Reject the Promise with the error
         reject(error);
      });

      // Handle connection close
      client.on('close', (error) => {
         if (error) {
            console.error('Connection to C server closed due to an error');
            // Handle the error appropriately
            reject(new Error('Connection error'));
         } else {
            console.log('Connection to C server closed');
            // Clean up any resources or perform necessary actions
            resolve(loginStatus); // Resolve the Promise with 1 (success)
         }
      });

   });
}

// Function to send register data to the C server
function tryRegister(socketAddress, socketPort, username, password) {
   return new Promise((resolve, reject) => {
      // Create a new socket connection to the C server
      const client = new net.Socket();
      let registerStatus = 0;
   
      // Connect to the C server
      client.connect(socketPort, socketAddress, () => {
         // Connection to C server established
         console.log('Connected to C server (Register)');

         // Send the data to the C server
         client.write('register\n'+username+'\n'+password);
      });

      // Handle data received from the server
      client.on('data', (data) => {
         console.log('Received data from server:', data.toString());
         if(data.toString() === '1'){
            registerStatus = 1;
         }else{
            registerStatus = 0;
         } 
         client.end();
         resolve(registerStatus); // Resolve the Promise with 1 (success)
      });
   
      // Handle connection error
      client.on('error', (error) => {
         console.error('Error connecting to C server:', error);
         // Close the socket connection
         client.destroy();
         // Reject the Promise with the error
         reject(error);
      });

      // Handle connection close
      client.on('close', (error) => {
         if (error) {
            console.error('Connection to C server closed due to an error');
            // Handle the error appropriately
            reject(new Error('Connection error'));
         } else {
            console.log('Connection to C server closed');
            // Clean up any resources or perform necessary actions
            resolve(registerStatus); // Resolve the Promise with 1 (success)
         }
      });

   });
}

// Function to send create group data to the C server
function tryCreateGroup(socketAddress, socketPort, username, groupName) {
   return new Promise((resolve, reject) => {
      // Create a new socket connection to the C server
      const client = new net.Socket();
      let createdGroup = 0;
   
      // Connect to the C server
      client.connect(socketPort, socketAddress, () => {
         // Connection to C server established
         console.log('Connected to C server (create)');

         // Send the data to the C server
         client.write('create\n'+groupName+'\n'+username);
      });

      // Handle data received from the server
      client.on('data', (data) => {
         console.log('Received data from server:', data.toString());
         if(data.toString() === '1'){
            createdGroup = 1;
         }else{
            createdGroup = 0;
         } 
         client.end();
         resolve(createdGroup); // Resolve the Promise with 1 (success)
      });
   
      // Handle connection error
      client.on('error', (error) => {
         console.error('Error connecting to C server:', error);
         // Close the socket connection
         client.destroy();
         // Reject the Promise with the error
         reject(error);
      });

      // Handle connection close
      client.on('close', (error) => {
         if (error) {
            console.error('Connection to C server closed due to an error');
            // Handle the error appropriately
            reject(new Error('Connection error'));
         } else {
            console.log('Connection to C server closed');
            // Clean up any resources or perform necessary actions
            resolve(createdGroup); // Resolve the Promise with 1 (success)
         }
      });

   });
}

// Function to send join group data to the C server
function tryJoinGroup(socketAddress, socketPort, username, groupName) {
   return new Promise((resolve, reject) => {
      // Create a new socket connection to the C server
      const client = new net.Socket();
      let groupJoined = 0;
   
      // Connect to the C server
      client.connect(socketPort, socketAddress, () => {
         // Connection to C server established
         console.log('Connected to C server (join)');

         // Send the data to the C server
         client.write('join\n'+groupName+'\n'+username);
      });

      // Handle data received from the server
      client.on('data', (data) => {
         console.log('Received data from server:', data.toString());
         if(data.toString() === '1'){
            groupJoined = 1;
         }else{
            groupJoined = parseInt(data.toString());
         } 
         client.end();
         resolve(groupJoined); // Resolve the Promise with 1 (success)
      });
   
      // Handle connection error
      client.on('error', (error) => {
         console.error('Error connecting to C server:', error);
         // Close the socket connection
         client.destroy();
         // Reject the Promise with the error
         reject(error);
      });

      // Handle connection close
      client.on('close', (error) => {
         if (error) {
            console.error('Connection to C server closed due to an error');
            // Handle the error appropriately
            reject(new Error('Connection error'));
         } else {
            console.log('Connection to C server closed');
            // Clean up any resources or perform necessary actions
            resolve(groupJoined); // Resolve the Promise with 1 (success)
         }
      });

   });
}
 

const app = express();
app.use(cors()); // Enable CORS for all routes

app.use(express.json()); // Parse JSON request bodies

const server = http.createServer(app);

port = 3000
server.listen(port, 'localhost', function () {
  console.log('HTTP server listening on port 3000');
});

// Test just NodeJSServer
app.get('/test', function (req, res) {
   const responseData = {
     status: 1
   };
 
   res.statusCode = 200;
   res.setHeader('Content-Type', 'application/json');
   res.json(responseData);
});

// Test NodeJS and Socket Server
app.post('/test', function (req, res) {
   // Access the data sent in the request body
   const requestData = req.body;

   console.log(requestData);

   const socketAddress = requestData.socketAddress;
   const socketPort = requestData.socketPort;

   testSocketServer(socketAddress, Number(socketPort))
      .then((result) => {
         let responseData;
         if(result){
            responseData = {
               status: true
            };
         }else{
            responseData = {
               status: false
            };
         }
         res.status(200).json(responseData);
         console.log('Connection successful:', result); // 1
      })
      .catch((error) => {
         const responseData = {
            status: false
         };
         res.status(200).json(responseData);
         console.error('Connection error:', error);
         // Handle the error appropriately
      });

});

// Login Request
app.post('/login', function (req, res) {
   // Access the data sent in the request body
   const requestData = req.body;

   console.log(requestData);

   const socketAddress = requestData.socketAddress;
   const socketPort = requestData.socketPort;
   const username = encryptString(requestData.username);
   const password = encryptString(requestData.password);

   tryLogin(socketAddress, Number(socketPort), username, password)
      .then((result) => {
         let responseData;
         if(result){
            responseData = {
               status: true
            };
         }else{
            responseData = {
               status: false
            };
         }
         res.status(200).json(responseData);
         console.log('Login:', result); // 1
      })
      .catch((error) => {
         let responseData;
         responseData = {
            status: false
         };
         res.status(200).json(responseData);
         console.error('connection error:', error);
         // Handle the error appropriately
      });
});

// Register Request
app.post('/register', function (req, res) {
   // Access the data sent in the request body
   const requestData = req.body;

   console.log(requestData);

   const socketAddress = requestData.socketAddress;
   const socketPort = requestData.socketPort;
   const username = encryptString(requestData.username);
   const password = encryptString(requestData.password);

   tryRegister(socketAddress, Number(socketPort), username, password)
      .then((result) => {
         let responseData;
         if(result){
            responseData = {
               status: true
            };
         }else{
            responseData = {
               status: false
            };
         }
         res.status(200).json(responseData);
         console.log('Register:', result); // 1
      })
      .catch((error) => {
         let responseData;
         responseData = {
            status: false
         };
         res.status(200).json(responseData);
         console.error('connection error:', error);
         // Handle the error appropriately
      });

});

// Create Group Request
app.post('/createGroup', function (req, res) {
   // Access the data sent in the request body
   const requestData = req.body;

   console.log(requestData);

   const socketAddress = requestData.socketAddress;
   const socketPort = requestData.socketPort;
   const username = encryptString(requestData.username);
   const groupName = encryptString(requestData.groupName);

   tryCreateGroup(socketAddress, Number(socketPort), username, groupName)
      .then((result) => {
         let responseData;
         if(result){
            responseData = {
               status: true
            };
         }else{
            responseData = {
               status: false
            };
         }
         res.status(200).json(responseData);
         console.log('created group:', result); // 1
      })
      .catch((error) => {
         let responseData;
         responseData = {
            status: false
         };
         res.status(200).json(responseData);
         console.error('connection error:', error);
         // Handle the error appropriately
      });
});

// Join Group Request
app.post('/joinGroup', function (req, res) {
   // Access the data sent in the request body
   const requestData = req.body;

   console.log(requestData);

   const socketAddress = requestData.socketAddress;
   const socketPort = requestData.socketPort;
   const username = encryptString(requestData.username);
   const groupName = encryptString(requestData.groupName);

   tryJoinGroup(socketAddress, Number(socketPort), username, groupName)
      .then((result) => {
         let responseData;
         if(result == 1){
            responseData = {
               status: true
            };
         }else{
            let message = '';
            if(result == 2){
               message = "Pending Request to Join";
            }else if(result == 3){
               message = "Already Member of Group";
            }else{
               message = "Group does not exist";
            }
            responseData = {
               status: false,
               message: message
            };
         }
         res.status(200).json(responseData);
         console.log('joined group:', result); // 1
      })
      .catch((error) => {
         let responseData;
         responseData = {
            status: false
         };
         res.status(200).json(responseData);
         console.error('connection error:', error);
         // Handle the error appropriately
      });

});


/*
client.connect(port, function () {
   console.log("Connected to server on port " + port);
   client.write('Hi from the client');
 });
 
 client.on('data', function (data) {
   console.log("Client 1 received from server: " + data);
 });
 
 client.on('close', function () {
   console.log('Client 1: Connection Closed');
 });
 
 client.on('error', function (error) {
   console.error("Connection Error: " + error);
 });  */