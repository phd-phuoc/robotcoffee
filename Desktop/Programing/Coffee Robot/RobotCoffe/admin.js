var socket;
socket = io.connect('http://localhost:3000');
var url = window.location.href;
var Users;
var Options;
var users_panel = document.getElementsByName("name")[0];
var editting_user;
function setup() {
  var canvas1 = createCanvas(900,400);
  background(51);
  canvas1.parent('rooms');

}


var code = getQueryVariable("code");
socket.emit("get-user-by-code",code);

socket.on('user-info',function (info) {
  alert(info.name);
  socket.emit('req-users',0);
});

socket.on('can-not-verify',function () {
  alert("Can not log in");
  var url = "http://localhost:3000/index.html";
  window.location.href = url;
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

      }
    }
  });
  socket.emit('get-order-list',1);
  setInterval(function() {
    socket.emit('get-order-list',0);
  },1000);

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
          '<h5><strong>Position:'+ +' </strong></h5>\n'+
          '<h5><strong>Order: '+list[ord].black +' </strong></h5>\n'+
          '<h5><strong>Done? </strong></h5>\n'+
          '<span><button type="button" id="btn-order-list-remove-1" class="btn btn-default"><span class="glyphicon glyphicon-remove"></span></button></span>\n'+
          '<span><button type="button" id="btn-order-list-ok-1" class="btn btn-default"><span class="glyphicon glyphicon-ok"></span></button></span>\n'+
        '</div>\n'+
      '</a>\n'+
    '</div>\n';
  }
});

///////////////////////////////////////////////////////////////////////////////

document.getElementById('create').addEventListener("click",function(){
  var newid = document.getElementById("addid").value;
  var newname = document.getElementById("addname").value;
  var newpw = document.getElementById("addpwd").value;

  if (newid!="" && newname!="" && newpw!="") {
    var new_user = {};
    new_user[newid] =
      {
      "user": newid,
      "pw": newpw,
      "name": newname,
      "code": "0"
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

  if (  newid!=Users[editting_user].user
     || newname!=Users[editting_user].name
     || newpw!=Users[editting_user].pw) {
       Users[editting_user].name = document.getElementById("name").value ;
       Users[editting_user].user = document.getElementById("id").value ;
       Users[editting_user].pw = document.getElementById("pwd").value ;
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

///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
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
