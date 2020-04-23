function insert_like(postId,this_){ 
	if(readCookie('Viewed' + postId) != postId)
		{    		
			var data = {
				"action" : "stoned_insert_likes",
				"post_id" : postId
			};
			
			jQuery.post(stonedthemes_Ajax.ajaxurl,data,function(data){	 
				if(data){
					
					this_.find('span').text(data);		
				}  
			});
			createCookie('Viewed' + postId,postId,365);  	
		}	
	}
function remove_like(postId,this_){ 
	if(readCookie('Viewed' + postId) == postId)
		{    		
			var data = {
				"action" : "stoned_remove_likes",
				"post_id" : postId
			};
			
			jQuery.post(stonedthemes_Ajax.ajaxurl,data,function(data){	 
				if(data){
					this_.find('span').text(data);		
				}  
			});
			eraseCookie('Viewed' + postId);  	
		}	
	}	
function insert_view(postId){ 
if(readCookie('Viewed' + postId) != postId)
	{    		
		var data = {
			"action" : "stoned_insert_likes",
			"post_id" : postId
		};		
		jQuery.post(stonedthemes_Ajax.ajaxurl,data,function(data){	   
			jQuery('.blogview').text(' '+data+'');			
		});
		createCookie('Viewed' + postId,postId,365);  	
	}	
}	
function createCookie(name, value, days) {
		if (days) {
			var date = new Date();
			date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
			var expires = "; expires=" + date.toGMTString();
		} else var expires = "";
		document.cookie = escape(name) + "=" + escape(value) + expires + "; path=/";
}

function readCookie(name) {
		var nameEQ = escape(name) + "=";
		var ca = document.cookie.split(';');
		for (var i = 0; i < ca.length; i++) {
			var c = ca[i];
			while (c.charAt(0) == ' ') c = c.substring(1, c.length);
			if (c.indexOf(nameEQ) == 0) return unescape(c.substring(nameEQ.length, c.length));
		}
		return null;
}

function eraseCookie(name) {
	createCookie(name, "", -1);
}