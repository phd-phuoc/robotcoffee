var url = 'http://localhost:3000';
var socket;
socket = io.connect(url);
//var url = window.location.href;
var black_no = 0;
var user;
var map=0;
var pos1x = [];
var pos1y = [];
var pos2x = [];
var pos2y = [];
var dirc = [];
var abs_dir = [];

var code = getQueryVariable("code");
socket.emit("get-user-by-code",code);

socket.on('user-info',function (info) {
  user = info;
  alert(info.name);
  document.getElementById('map-name').innerHTML = "ROOM "+info.room;
  socket.emit("req-map",info.room);
});

socket.on('can-not-verify',function () {
  alert("Can not log in");
  var uri = url+"/index.html";
  window.location.href = uri;
});
socket.on('send-map',function(_map) {
  map = _map;
  console.log("A");
  flag_map = 1;
});

///////////////////////////////////////////////////
document.getElementById("black-up").addEventListener("click",function() {
  black_no++;
  document.getElementById("black-no").textContent=black_no;
});

document.getElementById("black-down").addEventListener("click",function() {
  black_no--;
  document.getElementById("black-no").textContent=black_no;
});

document.getElementById("btn-confirm").addEventListener("click",function() {
  var order = {
    "name": user.name,
    "id":user.user,
    "pos":user.pos,
    "black":black_no,
  }
  socket.emit("send-order",order);
  alert("Order has been sent.");
});
//////////////////////////////////////////////////////////////////////////////
var flag_map = 0;
var flag_redraw = 0;
var flag_pos_valid = 0;
var delta_pos;
var pos1x_cp=[];
var pos1y_cp=[];
var pos2x_cp=[];
var pos2y_cp=[];
var dirc_cp=[];

function setup() {
  var width = 900;
  var height= 450;
  var canvas1 = createCanvas(width,height);
  background(51);
  canvas1.parent('room');

}

function draw(){
  if(flag_map == 1){
    redrawmap();
    drawPos(user.pos);
    //console.log("B");
  }
  if (flag_redraw==1){
    background(51);
    redrawmap();
    flag_redraw=0;
  }
}

///////////////////////////////////////////////////////////////////////
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

function redrawmap(){
  stroke(255, 204, 255);
  strokeWeight(4);
  pos1x_cp=[];
  pos1y_cp=[];
  pos2x_cp=[];
  pos2y_cp=[];
  dirc_cp=[];
  abs_dir_cp = [];
  drawmap(map.S,'S',width/2,height,0);
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
  if (curpos!=null){
    if (curpos.F !=null){
      if (dir==0) {x2 = x1; y2 = y1-curpos.F;}
      if (dir==1) {x2 = x1-curpos.F; y2 = y1;}
      if (dir==2) {x2 = x1; y2 = y1+curpos.F;}
      if (dir==3) {x2 = x1+curpos.F; y2 = y1;}
      pos1x_cp.push(x1); pos1y_cp.push(y1);
      pos2x_cp.push(x2); pos2y_cp.push(y2);
      dirc_cp.push(way);abs_dir_cp.push(dir);
      line(x1,y1,x2,y2);
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

/////////////////////////////////////////////////////////////////////////
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
