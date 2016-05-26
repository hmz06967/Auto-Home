var sensorbilgi = "?sensor_bilgisi_al=on",
	led1yak = "?led1durum",
	interval,
	sel="",
	value,
	efgv,
	lm35,
	time,
	say,
	derece,
	pot,
	esbldt="",
	analog="anlgsensor",
	yhizi=5,
	errnbr=1,
	getLed,
	adeg="",
	t,
	ass="autoSaves",
	verisync="onthe",
	sondf= "switchOn",
	tablemode = ["column","line","area","spline","pie"];
	
function saat(){
	var dt = new Date();
	var time = dt.getHours() + ":" + dt.getMinutes() + ":" + dt.getSeconds();
	return time;
}
function veriCekStart(edj,hiz){
	var syn=$(edj), veri = syn.hasClass(verisync);
	if(veri){syn.removeClass(verisync);clearInterval(interval);syn.html("Sync Başlat");}else{syn.addClass(verisync);verileriCevir( sensorbilgi,hiz);syn.html("Durdur");};
}	
function verileriCevir(get,hiz) {
   addClass("veri");if(hiz!=undefined)yhizi = hiz;
   interval = setInterval(function(){
   $.ajax({
	   url: get,
	   timeout: 5000,
	   success: function(data){
	   $("#lm35.analog-data").each(function(i,v){if(i>25)this.remove();});
		LocalKaydet(data_cer(data,false));},
	   error: function() { error("#error","","hata");$('#veri_cek').removeClass(verisync).html("Sync Başlat"); clearInterval(interval);return false;}}); }, yhizi*1000);
}

function led_ac(){
	var leds = $("#led1"),led= leds.hasClass(sondf);
	if(led){leds.removeClass(sondf);leddurum(false)}else{leds.addClass(sondf);leddurum(true)};	
}

function leddurum(durum){
	if(durum){getLed = led1yak+"=acik";}else{getLed= led1yak+"=kapali";remClass("led_on");}
    $.get(getLed,function(data){
		if(data=="ok"){remClass("led_hata");if(durum)addClass("led_on");}
		console.log(data);
	}).fail(function() {
		error(".led","","led_hata");remClass("led_on");
  });
  }
  
function addClass(c){
	$("body").addClass(c);
	
}  
function remClass(c){
	$("body").removeClass(c);
}  
function error(h,t,a){
	var h = ".error-main"+h;
	errnbr = parseInt($(h+" ._nbr").html());
	if(t=="undefined" || t=="")t=" Cihaz Bağlantısı Koptu"; 
	if(isNaN(errnbr))errnbr=1;else{errnbr++}
	t = '<span class="_nbr">'+errnbr+'</span> '+t;
	$(h).html(t);
	addClass(a);
}

function autoStartSave(edj){
	var leds = $(edj),led= leds.hasClass(sondf);
	if(led){leds.removeClass(sondf);localStorage.setItem("ass",false);}else{leds.addClass(sondf);if(supports_html5_storage())localStorage.setItem("ass",true);};
}
function TableSetOn(){
	if(adeg !="" && adeg !=undefined){
		var table = tableGet(adeg);
		$('[data-highcharts-chart]').remove();
		$("#temp_grap").html(table);
		$('table.highchart').highchartTable();
		addClass("on_grap");
	}
}

function supports_html5_storage(){
    try {
        return 'localStorage' in window && window['localStorage'] !== null;
    } catch(e) {
        return false;
    }
}
function AutoInputValue(edj){
	$(edj).keyup(function(){var hiz = $(this).val(); if(hiz>0){yhizi = hiz;if(supports_html5_storage())localStorage.setItem("yhizi",yhizi);
	clearInterval(interval);verileriCevir( sensorbilgi,hiz);}});
}
function LocalKaydet(deger){
	if(supports_html5_storage() && deger !=undefined){
	var jslo = localStorage.getItem(analog);
		if(typeof jslo && jslo !== null && jslo.length < 150){
				deger=jslo+deger; localStorage.setItem(analog,deger);
		}else if(jslo == null)localStorage.setItem(analog,deger);
		
	}
}
function Aktar(hangi){
	if(hangi){
		
		 if(supports_html5_storage() && adeg !== ""){
			var adeg = localStorage.getItem(analog);
			if(confirm("Excel çıktısı almak istiyor musunuz?")){
				esbldt = tableGet(adeg);
				window.open("data:application/vnd.ms-excel,<table><tr><th>Saat</th><th>Sicaklik</th></tr>"+esbldt+"</table>");				
			}
	 
		 }else{
			alert("Henüz Veri Yok");
		 }
	}
}
function tableGet(adeg){
	var split = adeg.split("-_-"); 
	$.each(split,function(i,data){
		if(i<50){
			data = data.split("__"); time = data[0]; der = data[1];
			if(der !=undefined) esbldt = esbldt+"<tr><td>"+time+"</td><td>"+der+"</td></tr>";
		}
	});
	return esbldt;	
}

function data_cer(data,slt){
	var time = saat(); data = data.split("__");lm35 = data[0]; pot = data[1]; 
	if(slt){time = data[0]; lm35 = data[1]; pot = 0;} derece =(lm35*500)/1023;	
	$(".analog-data").removeClass("select"); remClass("hata");if(lm35 !=undefined && time !=undefined){$("#lm35").prepend('<div class="analog-data select" id="lm35">'+time+'<span class="temp"> '+derece.toPrecision(3)+' C°</span></div>'); adeg = adeg+"-_-"+time+"__"+lm35;} $("#set-pot").text(pot);potProgress(pot);
	return adeg;
}
function Opens(edj){
	var href = $(edj).attr("href");
	window.open(href);
}
function potProgress(val,max){
	if(max==undefined)max=1023;
	val = 100/max*val;
	document.getElementById("pt_prog").style.width = val+"%";
}
function kapat(edj){
	var k =  $(edj).attr("data-kapat");
	remClass(k);
}
if(supports_html5_storage()){
	var gass = localStorage.getItem("ass"),yhizis= localStorage.getItem("yhizi"),adegs =  localStorage.getItem(analog);
	if(yhizis !== null){
		document.getElementById("yen_hiz").value = yhizis;
		yhizi = yhizis;
	}	
	if(gass=="true"){
		var d = document.getElementById("auto_st");d.className += " "+sondf;veriCekStart("#veri_cek",yhizi);
	}
	if(adegs != null){ 
		addClass("veri");
		var split = adegs.split("-_-"); 
		$.each(split,function(i,data){
			data_cer(data,true);
		});
	}
}