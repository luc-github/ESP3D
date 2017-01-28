var debug_enabled = false;

function get_uri_params() {
	// This function is anonymous, is executed immediately and
	// the return value is assigned to QueryString!
	var query_string = {};
	var query = window.location.search.substring(1);
	var vars = query.split("&");
	for (var i = 0; i < vars.length; i++) {
		var pair = vars[i].split("=");
			// If first entry with this name
		if (typeof query_string[pair[0]] === "undefined") {
			query_string[pair[0]] = decodeURIComponent(pair[1]);
			// If second entry with this name
		} else if (typeof query_string[pair[0]] === "string") {
			var arr = [ query_string[pair[0]],decodeURIComponent(pair[1]) ];
			query_string[pair[0]] = arr;
			// If third or later entry with this name
		} else {
			query_string[pair[0]].push(decodeURIComponent(pair[1]));
		}
	}
	return query_string;
};

function visible(id, is_visible) {
	var el = elm(id);
	if (!el)
		return;
	if (is_visible) {
		el.style.visibility = "visible";
		el.style.height = "";
		el.style.display = "";
	} else {
		el.style.visibility = "hidden";
		el.style.height = "0px";
		el.style.display = "none";
	}
}

function replace_html_array(el, data)
{
	var html_sv = el.innerHTML;
	var html_res = "";
	var list = "";

	for (var i = 0; i < data.length; i++) {
		list = JSON.stringify(data[i]);
		replace_html_by_json(el, list);
		html_res = html_res + el.innerHTML;
		el.innerHTML = html_sv;
	}

	el.innerHTML = html_res;
}

function replace_html_by_json(el, data) {
	debug(arguments.callee.name + ">" + data);

	var vals = JSON.parse(data);

	for (key in vals) {
		var value = vals[key];
		if (key != "") {
			if (el.innerHTML.indexOf("$" + key +"$") == "-1") {
				debug(arguments.callee.name + "<strong>>>> UNUSED " + key+" : "+value + "</strong>");
			} else {
				debug(arguments.callee.name + ">>>" + key+" : "+value);

				if (key.indexOf("[]") > 0 ) {
						var arr_el = elm("$" + key + "$");
						replace_html_array(arr_el, value);
				} else {
						el.innerHTML = el.innerHTML.replace(RegExp("\\$" + key + "\\$", "g"), value);
				}
			}
		}
	};

	var re = RegExp("\\$[^\\$]+\\$", "g");
	var result = el.innerHTML.match(re);
	if (result != null) {
		for (var i = 0; i < result.length; i++) {
				debug(arguments.callee.name + "<strong>>>> NOT SETTED " + result[i]) + "</strong>";
		}
	}

	var vals = JSON.parse(data, function(key, value) {
		if (key == "SERVICE_PAGE" && value != "") {
			eval(value);
		}
	});
}

function get_page_values_dispatch(jsonresponse) {
	var menu_body = elm("container_menu");
	var container_body = elm("container");

	replace_html_by_json(menu_body, jsonresponse);
	replace_html_by_json(container_body, jsonresponse);
	get_page_values_cb();
}

function get_page_values(url) {
	debug(arguments.callee.name + "> get url: " + url);
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
			get_page_values_dispatch(xmlhttp.responseText);
		}
	}
	xmlhttp.open("GET", url, false);
	xmlhttp.send();
}

function debug(info)
{
	var debug_info = elm("debug_info");
	if (!debug_info || !debug_enabled)
		return;

	debug_info.innerHTML += "<br>" + info;
}

function elm(element_name)
{
	var res = parent.document.getElementById(element_name);
	if (!res) {
		var iframe = parent.document.getElementById('content');
		if (!iframe)
			return null;
		var innerDoc = iframe.contentDocument || iframe.contentWindow.document;
		res = innerDoc.getElementById(element_name);
	}
	return res;
}

function container_resize() {
	var container = elm("content");
	var footer = elm("footer");
	var container_menu = elm('container_menu');
	if (!container || !footer || !container_menu)
		return;

	var width = container_menu.clientWidth
	var new_height = container.contentWindow.document.body.scrollHeight + 30;

	if (new_height - 30 != container.height)
		container.height = new_height;
	container.width = width;
	footer.width = width;

}

function on_frame_load()
{
	parent.on_container_load();
	on_page_load();
	container_resize();
}
