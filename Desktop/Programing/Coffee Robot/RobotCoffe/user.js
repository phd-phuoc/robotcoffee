var socket;
socket = io.connect('http://localhost:3000');
var url = window.location.href;

var code = getQueryVariable("code");
socket.emit("get-user-by-code",code);

socket.on('user-info',function (info) {
  alert(info.name);
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
