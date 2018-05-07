//var url = 'http://192.168.1.23:3000';
var url = window.location.href;
var socket;
socket = io.connect(getQueryVariable1());

var Users;
var Options;
var users_panel = document.getElementsByName("name")[0];
var users_panel1 = document.getElementsByName("name")[1];
var editting_user;

var maps = 0;
var curmap;
var curroom = 1;
var pos1x = [];
var pos1y = [];
var pos2x = [];
var pos2y = [];
var dirc = [];
var abs_dir = [];

var flag_set_pos = 0;
var cur_pos_name = '';

var code = getQueryVariable("code");
socket.emit("get-user-by-code",code);

socket.on('user-info',function (info) {
  alert(info.name);
  socket.emit('req-users',0);
  socket.emit("req-map",0);
});

socket.on('can-not-verify',function () {
  var uri = getQueryVariable1();
  alert("Can not log in");
  window.location.href = uri;
});

socket.on('send-users',function(users){
  Users = users;
  for (i = users_panel.options.length;i>=0; i--) {
    users_panel.options[i] = null;
  }
  for (var usr in Users) {
      var option = document.createElement("option");
      option.text = Users[usr].name;
      users_panel.add(option);
  }
  users_panel.addEventListener("click",function(){
    //alert(users_panel.value);
    for (var usr in Users) {
      if (users_panel.value == Users[usr].name){
        editting_user = usr;
        document.getElementById("name").value = Users[usr].name;
        document.getElementById("id").value = Users[usr].user;
        document.getElementById("pwd").value = Users[usr].pw;
        document.getElementById("room").value = Users[usr].room;
      }
    }
  });

  users_panel1.addEventListener("click",function(){
    if (users_panel1.value == "All users")
      for (var usr in Users) {
        if (Users[usr].room == curroom){
          redrawmap();
          drawPos(Users[usr].pos);
        }
      }
    else
      for (var usr in Users) {
        if (users_panel1.value == Users[usr].name){
          cur_pos_name = Users[usr].name;
          background(51);
          redrawmap();
          drawPos(Users[usr].pos);
        }
      }
  });

  socket.emit('get-order-list',1);
  setInterval(function() {
    socket.emit('get-order-list',0);
  },1000);

});

socket.on('send-map',function(_map) {
  maps = _map;
  curmap = maps.R1;
});

socket.on('scan-complete',function(){
  updatingMap = 0;
  alert("Scan completed");
});

socket.on('send-order-list',function (list) {
  document.getElementById('order-list-display').innerHTML = '';
  for (var ord in list ){
    document.getElementById('order-list-display').innerHTML+=
    '<div class="col-xs-6 col-md-3">\n'+
      '<a href="#" class="thumbnail">\n'+
        '<div class="order-list-person">\n'+
          '<h5><strong>Name: '+ list[ord].name +' </strong></h5>\n'+
          '<h5><strong>ID   : '+list[ord].id +' </strong></h5>\n'+
          '<h5><strong>Position:'+list[ord].pos +' </strong></h5>\n'+
          '<h5><strong>Order: '+list[ord].black +' </strong></h5>\n'+
          '<h5><strong>Done? </strong></h5>\n'+
          '<span><button type="button" id="btn-order-list-remove-1" class="btn btn-default"><span class="glyphicon glyphicon-remove"></span></button></span>\n'+
          '<span><button type="button" id="btn-order-list-ok-1" class="btn btn-default"><span class="glyphicon glyphicon-ok"></span></button></span>\n'+
        '</div>\n'+
      '</a>\n'+
    '</div>\n';
  }
});

socket.on('update-map',function(map_room){
  updatingMap = 1;
  flag_redraw = 1;
  curmap = map_room;
});

socket.on('current-pos',function(pos) {
  cur_pos = pos;
  flag_redraw = 1;
});

///////////////////////////////////////////////////////////////////////////////
document.getElementById('set-pos').addEventListener("click",function(){
  document.getElementById('alert-set-pos').innerHTML="Set postiton";
  flag_set_pos = 1;
});


document.getElementById('create').addEventListener("click",function(){
  var newid = document.getElementById("addid").value;
  var newname = document.getElementById("addname").value;
  var newpw = document.getElementById("addpwd").value;
  var newrm = document.getElementById("addroom").value;
  if (newid!="" && newname!="" && newpw!="" && newrm!="") {
    var new_user = {};
    new_user[newid] =
      {
      "user": newid,
      "pw": newpw,
      "name": newname,
      "code": "0",
      "room": newrm,
      "pos":""
      }
    socket.emit("add_new_user",new_user);
    alert("Added a new user");
    socket.emit('req-users',0);
  } else {
    alert("Fill all the blanks");
  }
});


document.getElementById('save').addEventListener("click",function(){
  var newid = document.getElementById("id").value;
  var newname = document.getElementById("name").value;
  var newpw = document.getElementById("pwd").value;
  var newrm = document.getElementById("room").value;

  if (  newid!=Users[editting_user].user
     || newname!=Users[editting_user].name
     || newpw!=Users[editting_user].pw
     || newrm!=Users[editting_user].room) {
       Users[editting_user].name = document.getElementById("name").value ;
       Users[editting_user].user = document.getElementById("id").value ;
       Users[editting_user].pw = document.getElementById("pwd").value ;
       Users[editting_user].room = document.getElementById("room").value ;
    socket.emit("change_user",Users);
    alert("Change a user");
    socket.emit('req-users',0);
  } else {
    alert("Nothing is changed");
  }
});

document.getElementById('remove').addEventListener("click",function(){
  var newid = document.getElementById("id").value;
  var newname = document.getElementById("name").value;
  var newpw = document.getElementById("pwd").value;

  for (var usr in Users) {
    if (newname == Users[usr].name){
      socket.emit("remove-user",Users[usr].name);
      alert(Users[usr].name+" has been removed");
      socket.emit('req-users',0);
    }
  }
});


document.getElementById('analyze').addEventListener("click",function(){
  if (confirm("Are you sure to analyze the room ?")){
    socket.emit('analyze-room',curroom);
    alert("Analyzing room "+curroom);
  }
});


document.getElementById('room1').addEventListener("click",function(){
  SetUserList(1);
  curmap = maps.R1;
  flag_redraw = 1;
  curroom = 1;
});
document.getElementById('room2').addEventListener("click",function(){
  SetUserList(2);
  curmap = maps.R2;
  flag_redraw = 1;
  curroom = 2;
});
document.getElementById('room3').addEventListener("click",function(){
  SetUserList("3");
  curmap = maps.R3;
  flag_redraw = 1;
  curroom = 3;
});
document.getElementById('room4').addEventListener("click",function(){
  SetUserList(4);
  curmap = maps.R4;
  flag_redraw = 1;
  curroom = 4;
});
document.getElementById('room5').addEventListener("click",function(){
  SetUserList("5");
  curmap = maps.R5;
  flag_redraw = 1;
  curroom = 5;
});
///////////////////////////////////////////////////////////////////////
var flag_map = 0;
var flag_redraw = 0;
var flag_pos_valid = 0;
var delta_pos;
var pos1x_cp=[];
var pos1y_cp=[];
var pos2x_cp=[];
var pos2y_cp=[];
var dirc_cp=[];
var x_start ,y_start;
var updatingMap = 0;
var cur_pos = "";

function setup() {
  var width = 900;
  var height= 450;
  var canvas1 = createCanvas(width,height);
  background(51);
  canvas1.parent('rooms');
  x_start = width/2;
  y_start = height;
}

function draw(){
  if(maps !=0){
    flag_map=1;
    redrawmap();
  }
  if (flag_redraw==1){
    background(51);
    redrawmap();
    if (cur_pos != "") drawPos(cur_pos);
    flag_redraw=0;
  }
  if(flag_map==1){
    getPos();
  }
}

///////////////////////////////////////////////////////////////////////

function mousePressed() {
  if (flag_pos_valid!=0 && flag_set_pos == 1){
    //console.log(dirc[flag_pos_valid-1]+abs(delta_pos));
    document.getElementById('alert-set-pos').innerHTML="";
    var strg = dirc[flag_pos_valid-1]+abs(delta_pos);
    alert("Position for "+cur_pos_name+" is "+strg);
    flag_pos_valid = 0;
    flag_set_pos = 0;

    var data = {
      "name": cur_pos_name,
      "pos": strg
    };
    socket.emit('set_pos',data);
  }
}

function updatePos(pos){
  var max_length = 0;
  var max_length_id = 0;
  for (var i = 0;i<dirc.length;i++){
    var same = pos.match(dirc[i]);
    if (same !=null)
      if (same.length>max_length) {
        max_length_id = i;
        //max_length = same.length;
      }
  }
  var strg = dirc[max_length_id];
  var abs_cdir = abs_dir[max_length_id];
  var dis = pos.substr(strg.length,pos.length);
  stroke(255, 255, 255);
  strokeWeight(10);
  var x,y = 0;
  // if (strg.charAt(strg.length-1)=='S') { x=pos1x[max_length_id]; y=pos1y[max_length_id]-dis;}
  // if (strg.charAt(strg.length-1)=='R') { x=pos1x[max_length_id]-(-dis); y=pos1y[max_length_id];}
  // if (strg.charAt(strg.length-1)=='L') { x=pos1x[max_length_id]-dis; y=pos1y[max_length_id];}
  if (abs_cdir==0) { x=pos1x[max_length_id]; y=pos1y[max_length_id]-dis;}
  if (abs_cdir==3) { x=pos1x[max_length_id]-(-dis); y=pos1y[max_length_id];}
  if (abs_cdir==1) { x=pos1x[max_length_id]-dis; y=pos1y[max_length_id];}
  if (abs_cdir==2) { x=pos1x[max_length_id]; y=pos1y[max_length_id]-(-dis);}
  point(x,y);
  //console.log(strg.charAt(strg.length-1));
}

function drawPos(pos){
  var max_length = 0;
  var max_length_id = 0;
  for (var i = 0;i<dirc.length;i++){
    var same = pos.match(dirc[i]);
    if (same !=null)
      if (same.length>max_length) {
        max_length_id = i;
        //max_length = same.length;
      }
  }
  var strg = dirc[max_length_id];
  var abs_cdir = abs_dir[max_length_id];
  var dis = pos.substr(strg.length,pos.length);
  stroke(200, 100, 200);
  strokeWeight(10);
  var x,y = 0;
  // if (strg.charAt(strg.length-1)=='S') { x=pos1x[max_length_id]; y=pos1y[max_length_id]-dis;}
  // if (strg.charAt(strg.length-1)=='R') { x=pos1x[max_length_id]-(-dis); y=pos1y[max_length_id];}
  // if (strg.charAt(strg.length-1)=='L') { x=pos1x[max_length_id]-dis; y=pos1y[max_length_id];}
  if (abs_cdir==0) { x=pos1x[max_length_id]; y=pos1y[max_length_id]-dis;}
  if (abs_cdir==3) { x=pos1x[max_length_id]-(-dis); y=pos1y[max_length_id];}
  if (abs_cdir==1) { x=pos1x[max_length_id]-dis; y=pos1y[max_length_id];}
  if (abs_cdir==2) { x=pos1x[max_length_id]; y=pos1y[max_length_id]-(-dis);}
  point(x,y);
  //console.log(strg.charAt(strg.length-1));
}
//show white dot on the trace when floating over the mouse
function getPos(){
  for (var i = 0;i<pos1x.length;i++){
    if (pos1y[i] == pos2y[i])
      if (((mouseX>pos1x[i] && mouseX<pos2x[i])||
          (mouseX<pos1x[i] && mouseX>pos2x[i]))&&
          mouseY-pos1y[i]<5 && mouseY-pos1y[i]> -5){
            stroke(255, 255, 255);
            strokeWeight(10);
            point(mouseX,pos1y[i]);
            flag_redraw = 1;
            if (flag_set_pos == 1) flag_pos_valid = i+1;
            delta_pos = -Math.round(abs(mouseX-pos1x[i]));
          }
    if (pos1x[i] == pos2x[i])
      if (((mouseY<pos1y[i] && mouseY>pos2y[i])||
          (mouseY>pos1y[i] && mouseY<pos2y[i])) &&
          mouseX-pos1x[i]<5 && mouseX-pos1x[i]> -5){
            stroke(255, 255, 255);
            strokeWeight(10);
            point(pos1x[i],mouseY);
            flag_redraw = 1;
            if (flag_set_pos == 1) flag_pos_valid = i+1;
            delta_pos = Math.round(abs(mouseY-pos1y[i]));
          }
  }
}
function redrawmap(){
  stroke(0, 255, 0);
  strokeWeight(10);
  point(x_start,y_start);
  stroke(255, 204, 255);
  strokeWeight(4);
  pos1x_cp=[];
  pos1y_cp=[];
  pos2x_cp=[];
  pos2y_cp=[];
  dirc_cp=[];
  abs_dir_cp = [];
  console.log(curmap);
  drawmap(curmap.S,'S',x_start,y_start,0);
  pos1x = pos1x_cp;
  pos1y = pos1y_cp;
  pos2x = pos2x_cp;
  pos2y = pos2y_cp;
  dirc = dirc_cp;
  abs_dir = abs_dir_cp;
}
//0: len 1:trai 2:xuong 3:phai
function drawmap(curpos,way,x1,y1,dir) {
  var x2,y2;
  if (curpos.hasOwnProperty('F')){
    if (curpos.F !=null){
      if (dir==0) {x2 = x1; y2 = y1-curpos.F;}
      if (dir==1) {x2 = x1-curpos.F; y2 = y1;}
      if (dir==2) {x2 = x1; y2 = y1+curpos.F;}
      if (dir==3) {x2 = x1+curpos.F; y2 = y1;}
      pos1x_cp.push(x1); pos1y_cp.push(y1);
      pos2x_cp.push(x2); pos2y_cp.push(y2);
      dirc_cp.push(way);abs_dir_cp.push(dir);
      line(x1,y1,x2,y2);
      if (updatingMap){
        if (x2<0) {x_start += -x2+3; flag_redraw = 1;}
        if (x2>width) {x_start -= x2-width+3; flag_redraw = 1;}
        if (y2<0) {y_start += -y2+3; flag_redraw = 1;}
        if (y2>height) {y_start -= y2-height+3; flag_redraw = 1;}
      } else {
        if (x2<0) {x_start -= 1; flag_redraw = 1;}
        if (x2>width) {x_start += 1; flag_redraw = 1;}
        if (y2<0) {y_start += 1; flag_redraw = 1;}
        if (y2>height) {y_start -= 1; flag_redraw = 1;}
      }
      if (curpos.L!=null) {
        if (dir==0) drawmap(curpos.L,way+'L',x2,y2,1);
        if (dir==1) drawmap(curpos.L,way+'L',x2,y2,2);
        if (dir==2) drawmap(curpos.L,way+'L',x2,y2,3);
        if (dir==3) drawmap(curpos.L,way+'L',x2,y2,0);
      }
      if (curpos.R!=null) {
        if (dir==0) drawmap(curpos.R,way+'R',x2,y2,3);
        if (dir==1) drawmap(curpos.R,way+'R',x2,y2,0);
        if (dir==2) drawmap(curpos.R,way+'R',x2,y2,1);
        if (dir==3) drawmap(curpos.R,way+'R',x2,y2,2);
      }
      if (curpos.S!=null) {
        if (dir==0) drawmap(curpos.S,way+'S',x2,y2,0);
        if (dir==1) drawmap(curpos.S,way+'S',x2,y2,1);
        if (dir==2) drawmap(curpos.S,way+'S',x2,y2,2);
        if (dir==3) drawmap(curpos.S,way+'S',x2,y2,3);
      }
      if (curpos.L==null && curpos.R==null && curpos.S==null){
        //console.log(way+curpos.F);
      }
    }
  }
}

function SetUserList(room) {
  for (i = users_panel.options.length;i>=0; i--) {
    //users_panel.options[i] = null;
    users_panel1.options[i] = null;
  }
  var option1 = document.createElement("option");
  option1.text = "All users";
  users_panel1.add(option1);
  for (var usr in Users) {
    if (Users[usr].room == room){
      //var option = document.createElement("option");
      var option1 = document.createElement("option");
      //option.text = Users[usr].name;
      option1.text = Users[usr].name;
      //users_panel.add(option);
      users_panel1.add(option1);
    }
  }
}

//http://www.example.com/index.php?id=1&image=awesome.jpg
function getQueryVariable(variable)
{
       var query = window.location.search.substring(1);
       var vars = query.split("&");
       for (var i=0;i<vars.length;i++) {
               var pair = vars[i].split("=");
               if(pair[0] == variable){return pair[1];}
       }
       return(false);
}

function getQueryVariable1()
{
       var query = window.location.href;
       var http = query.split("//");
       var vars = http[1].split("//");
       return("http://"+vars[0]);
	
}
