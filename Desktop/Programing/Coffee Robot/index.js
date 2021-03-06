//var exec = require(child_process);
var express = require('express');
var app = express();
var server = app.listen(3000);
var ip = require('ip');
//////////////////////////////////////////////////////////////////////
var SerialPort = require('serialport');
//var port = new SerialPort('/dev/ttyUSB0',9600);
var port = new SerialPort('/dev/ttyS0',9600);

var analyze_flag = 0;
var mapupdate_flag = 0;
var moving_flag = 0;//0:stdby 1:delivering 2:delivered 3:
var current_pos = "";

var stt_check=0;
var cup_flag = 0;
var arm_pause_flag = 0;

var receivedDt = "";
port.on('open', function () {
  port.on('data',function (data) {
    var dt = data.toString();
    
    if (dt.substr(dt.length-1)!='#') receivedDt += dt;
    else {
      console.log("RC: "+receivedDt);
      if (analyze_flag){ // analyze room
        if (receivedDt != '@'){
		  var str = receivedDt.substr(2,receivedDt.length);
		  if (!isNaN(str.substr(str.length-2,str.length))){
			updateMap(str,analyze_flag);
			mapupdate_flag = 1;
		  }
		  else
			{port.write("^");port.write('\n');} //resend
        }
        else {
          console.log("DONE");
          mapupdate_flag = 2;
          analyze_flag = 0;
        }
      } else if (moving_flag==1) {
          if (receivedDt =='%') {// delivered
            moving_flag = 2;
          }
          else{
            current_pos = receivedDt;
          }
      } else if (moving_flag==3) {
          if (receivedDt =='*') {//finish deliver
            moving_flag = 0;
          }
          else{
            current_pos = receivedDt;
          }
      } 
      
      if (receivedDt =='kk'){
		  port.write(":s");port.write('\n'); //print standby
		  stt_check = 0;
	  } else if (receivedDt =='dd'){
		  port.write(":w");port.write('\n'); //print delivering
		  stt_check = 0;
	  } else if (receivedDt =='rr'){
		  port.write(":r");port.write('\n'); //print returning
		  stt_check = 0;
	  } else if (receivedDt =='zz'){
		  port.write(":z");port.write('\n'); //print analizing
		  stt_check = 0;
	  } else if (receivedDt =='Robot is ready'){
		  moving_flag = 0;
	  }
		
      receivedDt = "";
    }
  });
});

/////////////////////////////////////////////////////////////////////
var order_list=[];
var order_list_change = 0;

app.use(express.static('RobotCoffe'));
console.log("Socket server is running");

port.write(":"+ip.address());port.write('\n');


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
  socket.on('test',function (data) {
    console.log(data);
    socket.emit('a','asdsad');
  });
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
    if (room == 1) map.R1 = {"S":{}};
    if (room == 2) map.R2 = {"S":{}};
    if (room == 3) map.R3 = {"S":{}};
    if (room == 4) map.R4 = {"S":{}};
    if (room == 5) map.R5 = {"S":{}};
    var timer = setInterval(function() {
      if (mapupdate_flag==1){
        if (room == 1) socket.emit('update-map',map.R1);
        if (room == 2) socket.emit('update-map',map.R2);
        if (room == 3) socket.emit('update-map',map.R3);
        if (room == 4) socket.emit('update-map',map.R4);
        if (room == 5) socket.emit('update-map',map.R5);
        mapupdate_flag = 0;
      }else if (mapupdate_flag == 2) {
        mapupdate_flag = 0;
        clearInterval(timer);
        socket.emit('scan-complete',0);
      }
    },1000);
  });
  
  
  socket.on('cup-ok',function() {
	  cup_flag = 1;
	  console.log("cup ok");
  });
  
  socket.on('cup-count',function(cnt) {
	  if (cnt==0) port.write(":a");
	  else if (cnt==1) port.write(":b");
	  else if (cnt==2) port.write(":c");
	  else if (cnt==3) port.write(":d");
	  else if (cnt==4) port.write(":e");
	  else if (cnt==5) port.write(":f");
	  port.write('\n');
  });
  
  socket.on('arm-pause',function() {
	  if (!arm_pause_flag){
		arm_pause_flag = 1;
        port.write(":p");
	    port.write('\n');
	    console.log("arm-pause");
	  } else {
		arm_pause_flag = 0;
        port.write(":u");
	    port.write('\n');
	    console.log("arm-u-pause");
	  }
  });
  
});

////////////////////////////////////////////////////////////////////////////
var order = [];
var sending_flag = 0;
setInterval(function() {
	
    if (order_list.length != 0 && moving_flag==0 && !arm_pause_flag && !sending_flag) {
	  for (var i = 0;i < order_list.length; i++)
		order[i] = order_list[i];
      io.sockets.emit('move-arm',order.length);
      console.log("Send order");
      moving_flag = 1;    
      sending_flag = 1;
    }
    
    if (cup_flag==1 && sending_flag) {
		sending_flag = 0;
		cup_flag = 0;
		console.log(order[0].pos);
		port.write(order[0].pos);
		port.write('\n');
		order_list.shift();
		order.shift();
		
		var timer_moving = setInterval(function() {
			if (moving_flag==1){
			  io.sockets.emit('current-pos',current_pos);
			}else if (moving_flag == 2) {
			  if (order.length){
				console.log(order[0].pos);
				port.write(order[0].pos);
				port.write('\n');
				console.log("Send order");
				moving_flag=1;
				order.shift();
				order_list.shift();
			  } else {
				moving_flag = 3;
				clearInterval(timer_moving);
				io.sockets.emit('moving-complete',0);
			  }
			  
			}
		},500);
      
	}
},1000);

/////////////////////////////////////////////////////////////////////////////////
/*
var order;
setInterval(function() {
	
    if (order_list.length != 0 && moving_flag==0 ) {
      order = order_list[0];
      io.sockets.emit('move-arm',0);
      console.log("Send order");
      moving_flag = 1;    
      
    }
    
    if (cup_flag==1) {
		cup_flag = 0;
		console.log(order.pos);
		port.write(order.pos);
		port.write('\n');
		
		var timer_moving = setInterval(function() {
        if (moving_flag==1){
          io.sockets.emit('current-pos',current_pos);
        }else if (moving_flag == 2) {
          order_list.shift();
          moving_flag = 3;
          clearInterval(timer_moving);
          io.sockets.emit('moving-complete',0);
        }
      },500);
	}
},1000);
*/

setInterval(function() {
	if (stt_check){
		port.write(":o");
		port.write('\n');
	} // print offline
	stt_check = 1;
 	
},5000);



////////////////////////////////////////////////////////////////////////////
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
        //console.log(map);
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
