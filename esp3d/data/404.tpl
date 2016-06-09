<HTML>
<HEAD>
<title>Redirecting...</title> 
</HEAD>
<BODY>
<CENTER>Unknown page - you will be redirected...
<BR><BR>
if not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>
<BR><BR>
<PROGRESS name='prg' id='prg'></PROGRESS>

<script>
var i = 0; 
var x = document.getElementById("prg"); 
x.max=5; 
var interval=setInterval(function(){
	i=i+1; 
	var x = document.getElementById("prg"); 
	x.value=i; 
	if (i>5) 
		{
		clearInterval(interval);
		window.location.href='/';
		}
	},1000);
</script>
</CENTER>
</BODY>
</HTML>

