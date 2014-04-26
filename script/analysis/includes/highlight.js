var currentnode = "";

var phrase_textcolor = "#f00";
var phrase_bgcolor = "#ff0";
var span_bgcolor = "#cfc";

function highlight(nodeid)
{
	textid_prefix = "text-" + nodeid + "-";
	$("[id ^= '" + textid_prefix + "']").css({"background-color": "#ff0", "color": "#f00"});
	rangeid_prefix = "range-" + nodeid + "-";
	$("[id ^= '" + rangeid_prefix + "']").css({"background-color": span_bgcolor});
	rule_src = desc[nodeid + "-src"];
	rule_trg = desc[nodeid + "-trg"];
	$('.rules').html(rule_src+ "<br/><br/>" + rule_trg);
}

function unhighlight(nodeid)
{
	textid_prefix = "text-" + nodeid + "-";
	$("[id ^= '" + textid_prefix + "']").css({"background-color": "", "color": ""});
	rangeid_prefix = "range-" + nodeid + "-";
	$("[id ^= '" + rangeid_prefix + "']").css({"background-color": ""});
}

$(function(){
	$(".text").hover(
		function()
		{
			spl = $(this).attr("id").split("-");
			currentnode = spl[1] + "-" + spl[2];
			highlight(currentnode);
			$(".rules").show();
		},
		function()
		{
			unhighlight(currentnode);
			$(".rules").hide();
		}
	);
	$('.text').mousemove(
		function(e){
			spl = $(this).attr("id").split("-");
			offset = $(this).offset();
			$('.rules').css({'left': "", 'right': ""});
			newleft = e.clientX + 20;
			ruleswidth = $('.rules').outerWidth()
			if(spl[4] == "src")
			{
				if(newleft + ruleswidth < $(window).width())
				{
					$('.rules').css({'top': offset.top - $('.rules').height() - 21, 'left': newleft, 'right': ""});
				}
				else
				{
					$('.rules').css({'top': offset.top - $('.rules').height() - 21, 'left': "", 'right': 0});
				}
			}
			else
			{
				if(newleft + ruleswidth < $(window).width())
				{
					$('.rules').css({'top': offset.top + $(this).height() + 5, 'left': newleft, 'right': ""});
				}
				else
				{
					$('.rules').css({'top': offset.top + $(this).height() + 5, 'left': "", 'right': 0});
				}
			}
		}
	);
	$('.text').click(
		function(e){
			if(currentnode in parentref)
			{
				unhighlight(currentnode);
				currentnode = parentref[currentnode];
				highlight(currentnode);
			}
			return false;
		}
	);
})
