var express = require('express');
var app = express();
var server = app.listen(3000);

var order_list=[];
var order_list_change = 0;

app.use(express.static('RobotCoffe'));


console.log("Socket server is running");
var socket = require('socket.io');
var io = socket(server);

var fs = require('fs');
var data = fs.readFileSync('data.json');
var users = JSON.parse(data);
//console.log(users);

// words.insert = 2;
// var data = JSON.stringify(words,null,2);
// fs.writeFile('data.json',data,function(){
//   console.log(words);
// });


io.sockets.on('connection',function (socket){
  console.log("New client: "+socket.conn.remoteAddress);

  socket.on('login',function (data) {
    //console.log(data);
    var logged_in = 0;
    for (var usr in users){
      if (users[usr].user == data.user && users[usr].pw == data.pw){
        console.log(data.user + " logged in.");
        logged_in=1;
        users[usr].code = rndStr();
        if (users[usr].name=="Administrator")
          socket.emit('admin-logged-in',users[usr]);
        else
          socket.emit('user-logged-in',users[usr]);
      }
    }
    if (logged_in==0) socket.emit('log-in-failed');
  });

  socket.on('get-user-by-code',function (code) {
    var verified = 0;
    for (var usr in users){
      if (users[usr].code == code && code!="0" ){
        socket.emit('user-info',users[usr]);
        if (users[usr].name != "Administrator")
          users[usr].code = "0";
        verified = 1;
      }
    }
    if (verified==0) socket.emit('can-not-verify',0);
  });

  socket.on('req-users',function(){
    socket.emit('send-users',users);
  });

  socket.on('add_new_user',function(new_user){
    console.log(new_user);
    for (usr in new_user){
      users[usr] = new_user[usr];
    }
    console.log(users);
    var data = JSON.stringify(users,null,2);
    fs.writeFile('data.json',data,function(){});
  });

  socket.on("change_user",function(data){
    users = data;
    data = JSON.stringify(users,null,2);
    fs.writeFile('data.json',data,function(){});
  });

  socket.on("remove-user",function(data){
    for (var usr in users){
      if (users[usr].name==data){
        delete users[usr];
        data = JSON.stringify(users,null,2);
        fs.writeFile('data.json',data,function(){});
      }
    }
  });

  socket.on("send-order",function(order) {
    order_list.push(order);
    order_list_change = 1;
    //console.log(order_list);
  });

  socket.on('get-order-list',function(data) {
    if (order_list_change==1 || data==1){
      socket.emit('send-order-list',order_list);
      order_list_change = 0;
      console.log("sent order list");
    }
  });

});

///////////////////////////////////////////////////////
function rndStr() {
    x=Math.random().toString(36).substring(7).substr(0,8);
    while (x.length!=8){
        x=Math.random().toString(36).substring(7).substr(0,8);
    }
    return x;
}

//MongoDB////////
// var MongoClient = require('mongodb').MongoClient;
// var url = "mongodb://localhost:27017/aaaaa";
// MongoClient.connect(url, function(err, db) {
//   if (err) throw err;
//   console.log("Database created!");
//   db.close();
// });
