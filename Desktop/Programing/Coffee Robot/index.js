//var exec = require(child_process);
var express = require('express');
var app = express();
var server = app.listen(3000);
///////////////////////////////////
var SerialPort = require('serialport');
var port = new SerialPort('/dev/ttyS0',9600);

var analyze_flag = 0;
var receivedDt = "";
port.on('open', function () {
  port.on('data',function (data) {
    var dt = data.toString;
    //console.log(data.toString());
    if (dt.substr(dt.length-1)!='#') receivedDt += dt;
    else {
      console.log(receivedDt);
      if (analyze_flag){
        if (receivedDt != '@')
          updateMap(receivedDt,analyze_flag);
        else console.log("DONE");
      }
    }
  });
});

/////////////////////////////////////////////////////////////////////
var order_list=[];
var order_list_change = 0;

app.use(express.static('RobotCoffe'));
console.log("Socket server is running");
var socket = require('socket.io');
var io = socket(server);

var fs = require('fs');
var data = fs.readFileSync('users.json');
var users = JSON.parse(data);
var data1 = fs.readFileSync('map.json');
var map = JSON.parse(data1);
//console.log(users);

// words.insert = 2;
// var data = JSON.stringify(words,null,2);
// fs.writeFile('users.json',data,function(){
//   console.log(words);
// });

// updateMap("SRL200",3);
//port.write("123");port.write('\n');

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
    fs.writeFile('users.json',data,function(){});
  });

  socket.on("change_user",function(data){
    users = data;
    data = JSON.stringify(users,null,2);
    fs.writeFile('users.json',data,function(){});
  });

  socket.on("remove-user",function(data){
    for (var usr in users){
      if (users[usr].name==data){
        delete users[usr];
        data = JSON.stringify(users,null,2);
        fs.writeFile('users.json',data,function(){});
      }
    }
  });

  socket.on("send-order",function(order) {
    order_list.push(order);
    order_list_change = 1;
    port.write(order.pos);port.write('\n');
    //console.log(order_list);
  });

  socket.on('get-order-list',function(data) {
    if (order_list_change==1 || data==1){
      socket.emit('send-order-list',order_list);
      order_list_change = 0;
      //console.log("sent order list");
    }
  });

  socket.on("req-map",function(id) {
    if (id==0)socket.emit('send-map',map);
    if (id==1)socket.emit('send-map',map.R1);
    if (id==2)socket.emit('send-map',map.R2);
    if (id==3)socket.emit('send-map',map.R3);
    if (id==4)socket.emit('send-map',map.R4);
    if (id==5)socket.emit('send-map',map.R5);
    //console.log(map);
  });

  socket.on('set_pos',function(Pos) {
    for (var usr in users){
      if (users[usr].name==Pos.name){
        users[usr].pos=Pos.pos;
        //console.log(users[usr].name+users[usr].pos);
        data = JSON.stringify(users,null,2);
        fs.writeFile('users.json',data,function(){});
      }
    }
  });

  socket.on('analyze-room',function(room) {
    port.write("$");port.write('\n');
    analyze_flag = room;
  });

});

///////////////////////////////////////////////////////
function updateMap(pos,room){
  var cur = 0;
  var curPos;
  if (room == 1) curPos = map.R1;
  if (room == 2) curPos = map.R2;
  if (room == 3) curPos = map.R3;
  if (room == 4) curPos = map.R4;
  if (room == 5) curPos = map.R5;
  var dis=0;
  while (pos[cur]!=null){
    if (pos[cur]=="L" || pos[cur]=="R" || pos[cur]=="S"){
      if (!curPos.hasOwnProperty(pos[cur])){
        curPos[pos[cur]] = {"F":0};
        curPos = curPos[pos[cur]];
        //console.log();
      } else {
        curPos = curPos[pos[cur]];
      }
    }
    else {
      var temp_dis = 1*pos.substr(cur,pos.length);
      if (temp_dis>dis) {
        dis = temp_dis;
        curPos.F = dis;
        console.log(map);
        data1 = JSON.stringify(map,null,2);
        fs.writeFile('map.json',data1,function(){});
      }

    }
    cur++;
  }

}

function sendMessage(message) {
  exec.execFile('./remote',
                ['-m',message],
                function(error,stdout,stderr) {
                });
}

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
