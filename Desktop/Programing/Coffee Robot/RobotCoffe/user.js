var socket;
socket = io.connect('http://localhost:3000');
var url = window.location.href;
var black_no = 0;
var user;

var code = getQueryVariable("code");
socket.emit("get-user-by-code",code);

socket.on('user-info',function (info) {
  user = info;
  alert(info.name);
});

socket.on('can-not-verify',function () {
  alert("Can not log in");
  var url = "http://localhost:3000/index.html";
  window.location.href = url;
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
    "black":black_no,
  }
  socket.emit("send-order",order);
  alert("Order has been sent.")
});

//////////////////////////////////////////////
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
