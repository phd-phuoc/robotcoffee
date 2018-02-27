var express = require('express');
var app = express();
var server = app.listen(3000);

app.use(express.static('RobotCoffe'));


console.log("Socket server is running");
var socket = require('socket.io');
var io = socket(server);

var fs = require('fs');
var data = fs.readFileSync('data.json');
var users = JSON.parse(data);

// words.insert = 2;
// var data = JSON.stringify(words,null,2);
// fs.writeFile('data.json',data,function(){
//   console.log(words);
// });

io.sockets.on('connection',function (socket){
  console.log("New client: "+socket.conn.remoteAddress);

  socket.on('login',function (data) {
    console.log(data);
    var logged_in = 0;
    for (var usr in users){
      if (users[usr].user == data.user && users[usr].pw == data.pw){
        console.log(data.user + " logged in.");
        console.log(users[usr]);
        logged_in=1;
        socket.emit('logged-in',users[usr]);
      }
    }
    if (logged_in==0) socket.emit('log-in-failed');
  });

  socket.on('get-user-by-code',function (code) {
    for (var usr in users){
      if (users[usr].code == code){
        socket.emit('user-info',users[usr]);
      }
    }
  });

});


//MongoDB////////
// var MongoClient = require('mongodb').MongoClient;
// var url = "mongodb://localhost:27017/aaaaa";
// MongoClient.connect(url, function(err, db) {
//   if (err) throw err;
//   console.log("Database created!");
//   db.close();
// });
