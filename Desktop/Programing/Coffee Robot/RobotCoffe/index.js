var url = 'http://localhost:3000';
var socket;
socket = io.connect(url);


document.getElementById("login-btn").addEventListener("click",function(req,res) {
  var user = document.getElementById('username').value;
  var pw = document.getElementsByName("password")[0].value;
  socket.emit('login',{"user":user, "pw":pw});
});

socket.on('user-logged-in',function (user) {
  var uri = url+"/user.html"+"?code="+user.code;
  window.location.href = uri;
  console.log(uri);
});

socket.on('admin-logged-in',function (user) {
  var uri = url+"/admin.html"+"?code="+user.code;
  window.location.href = uri;
  console.log(uri);
});

socket.on('log-in-failed',function () {
  alert("Username or password is wrong.");
});
