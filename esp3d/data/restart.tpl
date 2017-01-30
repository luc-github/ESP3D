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

	function dispatch(jsonresponse) {
		var data = jsonresponse;
		var vals = JSON.parse(data, function(key, value) {
			if (key == "status" && value == "alive") {
				i = x.max;
			}
		});
	}


	interval = setInterval(function(){
		var xmlhttp = new XMLHttpRequest();
		xmlhttp.onreadystatechange = function() {
			if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
				dispatch(xmlhttp.responseText);
			}
		}
		xmlhttp.open("GET", "/PING", true);
		xmlhttp.send();

		i=i+1;
		var x = document.getElementById("prg");
		x.value=i;
		if (i>x.max) {
			clearInterval(interval);
			top.window.location.href='/';
		}
	},1000);
</script>
</BODY>
</HTML>
