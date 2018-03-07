var socket;
socket = io.connect('http://localhost:3000');


document.getElementById("login-btn").addEventListener("click",function(req,res) {
  var user = document.getElementById('username').value;
  var pw = document.getElementsByName("password")[0].value;
  socket.emit('login',{"user":user, "pw":pw});
});

socket.on('user-logged-in',function (user) {
  var url = "http://localhost:3000/user.html"+"?code="+user.code;
  window.location.href = url;
  console.log(url);
});

socket.on('admin-logged-in',function (user) {
  var url = "http://localhost:3000/admin.html"+"?code="+user.code;
  window.location.href = url;
  console.log(url);
});

socket.on('log-in-failed',function () {
  alert("Username or password is wrong.");
});
