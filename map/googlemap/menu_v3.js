
function hasClass(target, name) {
    return target.className.match(new RegExp('(\\s|^)' + name + '(\\s|$)'));
}

function removeClass(target, name) {
    if (hasClass(target, name)) {
        target.className = target.className.replace(new RegExp('(\\s|^)' + name + '(\\s|$)'), ' ');
    }
}

function addClass(target, name) {
    if (!hasClass(target, name)) {
        target.className += " " + name;
    }
}

function MenuControl(map) {
    this.container = document.createElement("div");
    this.coordinate = null;
    this.map = map;
    this.sender = null
    this.isEnable = true;
    this.container.className = "menu_casing";
    this.hide();
    this.items = new Array();
    var self = this;
    google.maps.event.addListener(map, "click", function() {
        self.hide();
    });
    google.maps.event.addListener(map, "movestart", function() {
        self.hide();
    });
    google.maps.event.addListener(map, "zoom_changed", function() {
        self.hide();
    });
    google.maps.event.addListener(map, "dragstart", function() {
        self.hide();
    });
    this.container.index = 1;
    map.controls[google.maps.ControlPosition.TOP_RIGHT].push(this.container);
}

MenuControl.prototype.show = function(groupName) {
    this.container.style.display = "block";
    if (groupName) {
        for (var i = 0; i < this.items.length; i++) {
            if (groupName == this.items[i].groupName) {
                this.items[i].show();
            } else {
                this.items[i].hide();
            }
        }
    }
}

MenuControl.prototype.hide = function(groupName) {
    this.container.style.display = "none";
    if (groupName) {
        for (var i = 0; i < this.items.length; i++) {
            if (groupName == this.items[i].groupName) {
                this.items[i].hide();
            } else {
                this.items[i].show();
            }
        }
    }
}

MenuControl.prototype.enable = function() {
    this.isEnable = true;
}

MenuControl.prototype.disable = function() {
    this.isEnable = false;
}

MenuControl.prototype.isHide = function() {
    return (this.container.style.display == "none");
}

MenuControl.prototype.addItem = function(item) {
    item.host = this;
    this.items.push(item);
    this.container.appendChild(item.casing);
}

MenuControl.prototype.addSeparator = function(group) {
    var separator = group || new MenuSeparator();
    if (typeof group == "string") {
        separator = new MenuSeparator(group);
    }
    this.items.push(separator);
    this.container.appendChild(separator.casing);
}


function MenuSeparator(groupName) {
    this.groupName = groupName;
    this.casing = document.createElement("div");
    this.casing.className = "menu_separator";
}

MenuSeparator.prototype.show = function() {
    this.casing.style.display = "block";
}

MenuSeparator.prototype.hide = function() {
    this.casing.style.display = "none";
}

function MenuItem(options) {
    options = options || {};
    this.text = options.text || "";
    this.icon = options.icon;
    this.handler = options.handler;
    this.groupName = options.groupName;
    this.host = null; //宿主

    this.casing = document.createElement("div");
    this.casing.className = "menu_item";

    var menu_text = document.createElement("div");
    menu_text.className = "menu_text";
    var text_lable = document.createElement("span");
    text_lable.innerHTML = this.text;
    menu_text.appendChild(text_lable);
    this.casing.appendChild(menu_text);

    var self = this;
    if (this.icon) {
        var item_icon = document.createElement("div");
        item_icon.className = "menu_icon";
        item_icon.style.backgroundImage = "url(" + self.icon + ")";
        self.casing.appendChild(item_icon);
    }

    if (typeof self.handler == "function") {
        google.maps.event.addDomListener(self.casing, "click", function() {
            if (self.host) {
                self.handler(self.host.coordinate);
                self.host.hide();
            }
        });
    }
    google.maps.event.addDomListener(self.casing, "mouseover", function() {
        addClass(self.casing, "item_active");
    });

    google.maps.event.addDomListener(self.casing, "mouseout", function() {
        removeClass(self.casing, "item_active");
    });
}

MenuItem.prototype.show = function() {
    this.casing.style.display = "block";
}

MenuItem.prototype.hide = function() {
    this.casing.style.display = "none";
}

function showEv(result) {
	var requesttime2 = new Date().getTime();
	$("#searchTime").html("request time:" + ((requesttime2 - requesttime1) / 1000) + "seconds");
	if(typeof(result) == 'undefined' && result)
	{
		flage = false;
		alert("test fail");
	}
	var data = JSON.stringify(result);
	$("#log").val(data);
	var currentIndex = -1;
	var count = result.count;
	$("#count").val(count);
	$.each(result.evStations.evStation,function(idx,obj) {
		var pos = new google.maps.LatLng(obj.position.latitude, obj.position.longitude);
		var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "EV-Stand.ico" });
		marker.setMap(map);
		addClickHandler(obj.id, marker);
		markers.push(marker)
	});
	var drawtime = new Date().getTime();
	$("#drawTime").html("draw time:" + ((drawtime - requesttime2) / 1000) + "seconds");
	if($("#vp").val() != 0 || $("#dp").val() == 0) {
		var vp_value_temp = $("#vp").val();
		var vp_temp = vp_value_temp.split(",");
		search_lng = vp_temp[1];
		search_lat = vp_temp[0];
		search_flag = 1;
	} else if($("#dp").val() != 0 || $("#vp").val() == 0) {
		var dp_value_temp = $("#dp").val();
		var dp_temp = dp_value_temp.split(",");
		search_lng = dp_temp[1];
		search_lat = dp_temp[0];
		search_flag = 2;				
	}
	request_time_value = ((requesttime2 - requesttime1) / 1000) + "seconds";
	draw_time_value = ((drawtime - requesttime2) / 1000) + "seconds";
}

function addClickHandler(content, marker) {
	google.maps.event.addListener(marker, "click", function(e) {
		openInfo(content,e);
	});
}

function openInfo(content,e) {
	var p = e.target;
	var src = "https://ev-v2.cit.cc.api.here.com/ev/stations/" + content + ".json?app_id=2XfHBClGvrrZVuOgk856&app_code=eAhDQZqHpp-FiTAuvnXSdA";
	$("#url").val(src);
	$("head").append("<script src='" + src + "&jsoncallback=showEvDetail'><\/script>");
}

function showEvDetail(result) {
	var name = 0;
	var lastUpdateTimestamp = 0;
	var address = 0;
	//var brand = 0;
	var lat = 0;
	var lng = 0;
	var totalNumberOfConnectors = 0;
	var connectorType_name = 0;
	var powerFeedType = 0;
	var numberOfConnectors = 0;
	var customerConnectorName = 0;
	var contacts = 0;
	var data = JSON.stringify(result);
	$("#log").val(data);
	$.each(result.evStations.evStation, function(idx, obj) {
		name = obj.name;
		lastUpdateTimestamp = obj.lastUpdateTimestamp;
		address = obj.address.streetNumber + obj.address.street + obj.address.region + obj.address.country + obj.address.city;
		totalNumberOfConnectors = obj.totalNumberOfConnectors;
		lat = obj.position.latitude;
		lng = obj.position.longitude;
		$.each(obj.connectors.connector, function(idx, obj1) {
			connectorType_name = obj1.connectorType.name;
			powerFeedType = obj1.powerFeedType.name;
			customerConnectorName = obj1.customerConnectorName;
			numberOfConnectors = obj1.chargingPoint.numberOfConnectors;
		});
		/*$.each(obj.contacts.phone, function(idx, obj2) {
			contacts = obj2.value;
		});*/
	});

	var infowindow = new google.maps.InfoWindow();
	infowindow.setContent('name:' + name + '<br/>' + 'lastUpdateTimestamp:' + lastUpdateTimestamp + '<br/>' + 'address:' + address + '<br/>' + 'totalNumberOfConnectors:' + totalNumberOfConnectors + '<br/>'
							+ 'connectorType:' + connectorType_name + '<br/>' + 'powerFeedType:' + powerFeedType + '<br/>' + 'customerConnectorName:' +
							customerConnectorName + '<br/>' + 'numberOfConnectors:' + numberOfConnectors );//+ '<br/>' +'contacts:' + contacts);
	infowindow.setPosition(new google.maps.LatLng(lat, lng));
	infowindow.open(map);
}


function init_map()
{	
	if (markers.length)
	{
		for (i=0; i < markers.length ; i++) 
		{
			markers[i].setMap(null);
		}			
	}
	if(polygon.length) {
		for(i = 0; i < polygon.length; i++) {
			polygon[i].setMap(null);
		}
	}
	if(flightPath_t.length) {
		for(i = 0; i < flightPath_t.length; i++) {
			flightPath_t[i].setMap(null);
		}
	}
	$("#url").val("");
	$("#log").val("");
	$("#searchTime").html("");
	$("#drawTime").html("");
	$("#selectmap").val("");
	$("#vp").val("");
	$("#dp").val("");
	$("#radius").val("5000");
	$("#rangevalue").val("4000");
	$("#resolution").val("50");
	$("#corridorWidth").val("5");
	$("#ascent").val("3.7");
	$("#descent").val("2.4");
	$("#MaxPoints").val("500");
	$("#timePenalty").val("0");
	$("#auxiliaryconsumption").val("0");
	$("#acceleration").val("2.4");
	$("#deceleration").val("1.5");
	$("#limitedWeight").val("0");
	$("#height").val("0");
	$("#width").val("0");
	$("#length").val("0");
	$("#test_count").val("0");
	$("#mode_select").val("fastest");
	$("#rangetype_select").val("distance");
	$("#routingtypes").val("car");
	$("#shippedHazardousGoods_select").val("explosive");
	$("#model_select").val("C-SDN")
	$("#speed").val("10,1.78,20,1.44,30,1.19,40,1.04,50,1.00,60,1.06,70,1.22,80,1.48,90,1.84,100,2.31,110,2.87,120,3.54");

	identity = 0;
	lat = 0;
	lng = 0;
	locationLat = 0;
	locationLng = 0;
	destinationLat = 0;
	destinationLng = 0;
			
	vp_value = 0;
	vp_value_lng = 0;
	vp_value_lat = 0;
	dp_value = 0;
	dp_value_lng = 0;
	dp_value_lat = 0;
	Mode_value = "fastest";
	rangetype_value = "distance";
	log_value = 0;
	range_value = 4000;
	test_count = 1;
	ascent_value = 0;
	descent_value = 0;
	timePenalty_value = 0;
	auxiliaryconsumption_value = 0;
	acceleration_value = 0;
	deceleration_value = 0;
	pointsring="";
	flage =true;
	request_time_value = 0;
	draw_time_value = 0;
	radius_value = 5000;
	maxpoints_value = 500;
	traffic_value = "disabled";
	resolution_value = 50;
	corridorWidth_value = 5000;
	limitedWeight_value = 0;
	routingtypes_value = "car";
	height_value = 0;
	width_value = 0;
	length_value = 0;
	shippedHazardousGoods_value = "explosive";
	operating_value = 0;
	speed_value=new Array(10,1.78,20,1.44,30,1.19,40,1.04,50,1.00,60,1.06,70,1.22,80,1.48,90,1.84,100,2.31,110,2.87,120,3.54);
	search_lng = 0;
	search_lat = 0;
	search_flag = 0;	
	requesttime1 = 0;	
	map.setCenter({lat:40.773358,lng:-73.958229}); 
	new_button = 0;
	document.getElementById("route_search").disabled = true;
	//polygon1.setMap(null);
}

function remove_overlay()
{
	if(polygon.length) {
		for(i = 0; i < polygon.length; i++) {
			polygon[i].setMap(null);
		}
	}
	if(flightPath_t.length) {
		for(i = 0; i < flightPath_t.length; i++) {
			flightPath_t[i].setMap(null);
		}
	}
	if(markers.length) {
		for(i = 0; i < markers.length; i++) {
			markers[i].setMap(null);
		}
	}

	$("#url").val("");
	$("#log").val("");
	$("#searchTime").html("");
	$("#drawTime").html("");
	$("#selectmap").val("");
	getValue(this);
	new_button = 0;
	pointsring="";
	if(vp_value_lat != '' && vp_value_lng != '')
	{
		var pos = new google.maps.LatLng(vp_value_lat, vp_value_lng);
		var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "location.png" });
		marker.setMap(map);
		markers.push(marker)			
	}
	if(dp_value_lat != '' && dp_value_lng != '')
	{
		var pos = new google.maps.LatLng(dp_value_lat, dp_value_lng);
		var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "destination.png" });
		marker.setMap(map);
		markers.push(marker);
	}
	$("#count").val("");
	document.getElementById("route_search").disabled = true;
}

function getValue(input) {
	range_value = $("#rangevalue").val();
	corridorWidth_value = $("#corridorWidth").val() * 1000;
	test_count = $("#test_count").val();
	ascent_value = $("#ascent").val();
	descent_value = $("#descent").val();
	resolution_value = $("#resolution").val();
	timePenalty_value = $("#timePenalty").val();
	auxiliaryconsumption_value = $("#auxiliaryconsumption").val();
	acceleration_value = $("#acceleration").val();
	deceleration_value = $("#deceleration").val();
	vp_value = $("#vp").val();
	dp_value = $("#dp").val();
	radius_value = $("#radius").val();
	speed_value = $("#speed").val();
	maxpoints_value = $("#MaxPoints").val();
	limitedWeight_value = $("#limitedWeight").val();
	height_value = $("#height").val();
	width_value = $("#width").val();
	length_value = $("#length").val();
	var vp_temp = vp_value.split(",");
	vp_value_lng = vp_temp[1];
	vp_value_lat = vp_temp[0];
	locationLng = vp_value_lng;
	locationLat = vp_value_lat;
	var dp_temp = dp_value.split(",");
	dp_value_lng = dp_temp[1];
	dp_value_lat = dp_temp[0];
	destinationLng = dp_value_lng;
	destinationLat = dp_value_lat;
}


function showIso(result) { //等值线
	var data = JSON.stringify(result);
	$("#log").val(data);
	var requesttime2 = new Date().getTime();
	$("#searchTime").html("request time:" + ((requesttime2 - requesttime1) / 1000) + "seconds");
	var isopoints = [];
	$.each(result.response.isoline, function(idx, obj) 
	{
		$.each(obj.component, function(idx, obj1)
		{
			$.each(obj1.shape, function(idx, obj2)
			{
				var iso_arr = obj2.split(",");
				var isolat = iso_arr[0];
				var isolng = iso_arr[1];
				var point = new google.maps.LatLng(isolat, isolng);
				isopoints.push(point);
			})
		})
	});
	var polygonOptions = {
		path: isopoints,
		fillColor: "white",
		fillOpacity: 0.3,
		strokeColor: "blue",
		strokeOpacity: 0.7,
		strokeWeight: 2
	};
	polygon1 = new google.maps.Polygon(polygonOptions);
	polygon1.setMap(map);
	polygon.push(polygon1);
	var drawtime = new Date().getTime();
	$("#drawTime").html("draw time:" + ((drawtime - requesttime2) / 1000) + "seconds");
	request_time_value = ((requesttime2 - requesttime1) / 1000) + "seconds";
	draw_time_value = ((drawtime - requesttime2) / 1000) + "seconds";
}

function showRoute(result) //算路
{
	var requesttime2 = new Date().getTime();
	$("#searchTime").html("request time:" + ((requesttime2 - requesttime1) / 1000) + "seconds");
	var flightPlanCoordinates = [];
	var consumption = $("#rangevalue").val();
	$.each(result.response.route,function(idx,obj)
	{
		$.each(obj.leg,function(idx,obj1)
		{
			$.each(obj1.maneuver,function(idx,obj2)
			{
				var pos = new google.maps.LatLng(obj2.position.latitude, obj2.position.longitude);
				flightPlanCoordinates.push(pos);
				pointsring += obj2.position.latitude+","+obj2.position.longitude+";";
			})
			
			$.each(obj1.link,function(idx,obj2)
			{
				//var pos = new google.maps.LatLng(obj2.shape[0], obj2.shape[1]);
				//flightPlanCoordinates.push(pos);
				consumption = consumption - obj2.consumption;
				if(consumption <= 0)
				{
					var pos = new google.maps.LatLng(obj2.shape[0], obj2.shape[1]);
					var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "empty.ico" });
					marker.setMap(map);
					markers.push(marker)
					return false;
				}
			})
		})
	});
	flightPath = new google.maps.Polyline({
		path: flightPlanCoordinates,
		geodesic: true,
		strokeColor: "blue",
		strokeOpacity: 1.0,
		strokeWeight: 4
	});
	flightPath.setMap(map);
	flightPath_t.push(flightPath);
	var drawtime = new Date().getTime();
	$("#drawTime").html("draw time:" + ((drawtime - requesttime2) / 1000) + "seconds");
	var data = JSON.stringify(result);
	$("#log").val(data);
	document.getElementById("route_search").disabled = false;
	request_time_value = ((requesttime2 - requesttime1) / 1000) + "seconds";
	draw_time_value = ((drawtime - requesttime2) / 1000) + "seconds";

}

function showRouteSearch(result) 
{
	var requesttime2 = new Date().getTime();
	$("#searchTime").html("request time:" + ((requesttime2 - requesttime1) / 1000) + "seconds");
	$("#count").val(result.count);
	$.each(result.evStations.evStation,function(idx,obj)
	{
		var pos = new google.maps.LatLng(obj.position.latitude, obj.position.longitude);
		var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "EV-Stand.ico" });
		marker.setMap(map);
		addClickHandler(obj.id, marker);
		markers.push(marker)
	});
	var drawtime = new Date().getTime();
	$("#drawTime").html("draw time:" + ((drawtime - requesttime2) / 1000) + "seconds");
	var data = JSON.stringify(result);
	$("#log").val(data);
	request_time_value = ((requesttime2 - requesttime1) / 1000) + "seconds";
	draw_time_value = ((drawtime - requesttime2) / 1000) + "seconds";
}

function start_test() {
	var count = 1;
	getValue(this);
	if(test_count == "" || 0 == test_count)
	{
		return false;	
	}
	while(flage)
	{
		requesttime1 = new Date().getTime();
		var src = "https://ev-v2.cit.cc.api.here.com/ev/stations.json?app_id=DemoCredForAutomotiveAPI&app_code=JZlojTwKtPLbrQ9fEGznlA&prox=" + locationLat + "," + locationLng + ",5000";
		$("#url").val(src);
		$("head").append("<script src='" + src + "&jsoncallback=showEv'><\/script>");
		if(test_count == count)
		{
			flage = false;
		}
		count++;
	}
	flage = true;
}

function vehiclepoint() {	
	getValue(this);
	var pos = new google.maps.LatLng(vp_value_lat, vp_value_lng);
	var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "location.png" });
	marker.setMap(map);
	map.setCenter(marker.getPosition()); 
	markers.push(marker)
}
function designatedpoint() {
	getValue(this);
	var pos = new google.maps.LatLng(dp_value_lat, dp_value_lng);
	var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "destination.png" });
	marker.setMap(map);
	map.setCenter(marker.getPosition()); 
	markers.push(marker)
}

function saveMapFile() {
	const base64 = (s)=> {
	  return window.btoa(unescape(encodeURIComponent(s)));
	}
	var log_text = $("#log").val();
	const str= log_text;
	const uri = 'data:text/plain;charset=UTF-8;base64,';
	const url = uri + base64(str);
	const a = document.createElement('a');
	a.href = url;
	//a.download = displayisoline_value + ";" + search_value + ";" + route_start_value + ";" + route_search_value + ";" + "Mode_" + Mode_value + ";" +"rangetype_" + rangetype_value + ";"+ "request_time" + request_time_value + ";" + "draw_time " + draw_time_value+".json";
	if(operating_value == "Station search") {
		a.download =search_lat + ";" + search_lng + ";" + search_flag + ";" + operating_value + ";" + "Radius_" + radius_value + ";" +"request-time_" + request_time_value + ";" + "draw-time_" + draw_time_value + ".json";
	} else if(operating_value == "displayisoline") {
		a.download = operating_value + ";" + "Mode_" + Mode_value + ";" + "Types_" + routingtypes_value + ";" + "traffic_" + traffic_value + ";" + "RangeType_" + rangetype_value + ";" + "Range_" + range_value + ";" + "MaxPoints_" + maxpoints_value + ";" + "Resolution_" + resolution_value + ";" + "ascent_" + ascent_value + ";" + "descent_" + descent_value + ";" + "request-time_" + request_time_value + ";" + "draw-time_ " + draw_time_value + ".json";
	} else if(operating_value == "CalculatingRoute") {
		a.download = operating_value + ";" + "Mode_" + Mode_value + ";" + "Types_" + routingtypes_value + ";" + "traffic_" + traffic_value + ";" +  "RangeType_" + rangetype_value + ";" + "request-time_" + request_time_value + ";" + "draw-time_ " + draw_time_value +".json";
	} else if(operating_value == "ChargingPileRoad") {
		a.download = operating_value + ";" + "CorridorWidth_" + corridorWidth_value + ";" + "request_time" + request_time_value + ";" + "draw_time " + draw_time_value + ".json";
	}
	a.click();
}

function loadMapFile(input) {
	if(window.FileReader) {
		var file = input.files[0];
		var reader = new FileReader();
		reader.readAsText(file);
		reader.onload = function() {
			$("#log").val(this.result);
		}
	}
}

function repaintingMap() {
	log_value = $("#log").val();
	var file = $("#selectmap").val();
	var pos = file.lastIndexOf("\\");
	var filename = file.substring(pos + 1);
	var filename_type = filename.split(";");
	var display_type = 0;
	search_flag = filename_type[2];
	if( !isNaN(filename_type[0]) && !isNaN(filename_type[1]) ) {
		search_lat = filename_type[0];
		search_lng = filename_type[1];
	}
	for(var i = 0; i < 4; i++){
		if(filename_type[i] == "displayisoline") {
			display_type = "displayisoline";
			break;
		} else if(filename_type[i] == "Station search") {
			display_type = "search";
			break;
		} else if(filename_type[i] == "CalculatingRoute") {
			display_type = "CalculatingRoute";
			break;
		} else if(filename_type[i] == "ChargingPileRoad"){
			display_type = "ChargingPileRoad";
			break;
		}
	}
	var obj_json = JSON.parse(log_value);
	if(display_type == "displayisoline") {
		var isopoints = [];
		var center_lng = 0;
		var center_lat = 0;
		center_lng = obj_json.response.center.longitude;
		center_lat = obj_json.response.center.latitude;
		var coor = new google.maps.LatLng(center_lat, center_lng);
		var marker = new google.maps.Marker({ position: coor, map: map, draggable: false, icon: "location.png" });
		marker.setMap(map);
		markers.push(marker);
		
		$.each(obj_json.response.isoline, function(idx, obj) 
		{
			$.each(obj.component, function(idx, obj1)
			{
				$.each(obj1.shape, function(idx, obj2)
				{
					var iso_arr = obj2.split(",");
					var isolat = iso_arr[0];
					var isolng = iso_arr[1];
					var point = new google.maps.LatLng(isolat, isolng);
					isopoints.push(point);
				})
			})
		});
		var polygonOptions = {
			path: isopoints,
			fillColor: "white",
			fillOpacity: 0.3,
			strokeColor: "blue",
			strokeOpacity: 0.7,
			strokeWeight: 2
		};
		polygon1 = new google.maps.Polygon(polygonOptions);
		polygon1.setMap(map);
		polygon.push(polygon1);
	}
	if(display_type == "CalculatingRoute") {
		var flightPlanCoordinates = [];
		$.each(obj_json.response.route,function(idx,obj)
		{
			$.each(obj.leg,function(idx,obj1)
			{
				$.each(obj1.maneuver,function(idx,obj2)
				{
					var pos = new google.maps.LatLng(obj2.position.latitude, obj2.position.longitude);
					flightPlanCoordinates.push(pos);
				})
				
				var center_lng = obj1.start.mappedPosition.longitude;
				var center_lat = obj1.start.mappedPosition.latitude;
				var pos = new google.maps.LatLng(center_lat, center_lng);
				var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "location.png" });
				marker.setMap(map);
				markers.push(marker);
				map.setCenter(marker.getPosition()); 
				
				
				center_lng = obj1.end.mappedPosition.longitude;
				center_lat = obj1.end.mappedPosition.latitude;
				var pos = new google.maps.LatLng(center_lat, center_lng);
				var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "destination.png" });
				marker.setMap(map);
				markers.push(marker);	
			})
		});
		flightPath = new google.maps.Polyline({
			path: flightPlanCoordinates,
			geodesic: true,
			//strokeColor: '#FF0000',
			strokeColor: "blue",
			strokeOpacity: 1.0,
			strokeWeight: 2
		});
		flightPath.setMap(map);
		flightPath_t.push(flightPath);

		var data = JSON.stringify(obj_json);
		$("#log").val(data);
	}
	if(display_type == "ChargingPileRoad") {
		$.each(obj_json.evStations.evStation,function(idx,obj)
		{
			var pos = new google.maps.LatLng(obj.position.latitude, obj.position.longitude);
			var marker = new google.maps.Marker({ position: pos, map: map, draggable: false, icon: "EV-Stand.ico" });
			marker.setMap(map);
			markers.push(marker);
			
		});
		var data = JSON.stringify(obj_json);
		$("#log").val(data);
	}
	if(display_type == "search") {
		var coor = new google.maps.LatLng(search_lat, search_lng);
		if(search_flag == 1) {
			var marker = new google.maps.Marker({ position: coor, map: map, draggable: false, icon: "location.png" });
			marker.setMap(map);
			markers.push(marker);
			
		} else if(search_flag == 2) {
			var marker = new google.maps.Marker({ position: coor, map: map, draggable: false, icon: "destination.png" });
			marker.setMap(map);
			markers.push(marker);
		}
		$.each(obj_json.evStations.evStation,function(idx,obj){
			var coor = new google.maps.LatLng(obj.position.latitude, obj.position.longitude);
			var marker = new google.maps.Marker({ position: coor, map: map, draggable: false, icon: "EV-Stand.ico" });
			marker.setMap(map);
			addClickHandler(obj.id, marker);
			markers.push(marker);
		});
		var data = JSON.stringify(obj_json);
		$("#log").val(data);
	}
	
}














