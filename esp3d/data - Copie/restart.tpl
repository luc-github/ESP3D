<HTML>
<HEAD>
<title>Restarting...</title> 
</HEAD>
<BODY>
<CENTER>Restarting, please wait....
<BR>
<PROGRESS name='prg' id='prg'></PROGRESS>
</CENTER>
<script>
var i = 0;
var interval; 
var x = document.getElementById("prg"); 
x.max=40; 
interval = setInterval(function(){
	i=i+1; 
	var x = document.getElementById("prg"); 
	x.value=i; 
	if (i>40) 
		{
		clearInterval(interval);
		window.location.href='/';
		}
	},1000);
</script>
</BODY>
</HTML>
