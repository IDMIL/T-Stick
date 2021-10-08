/*
*	fit-to-screen
*/
(function($){
	"use strict";
	var setFitScreenHeight = function(){
		if($(".fit-to-screen").length){
			var $this = $(".fit-to-screen");
			var wh = $(window).height();
			var hh = $('header').outerHeight();
			var fh = $('footer').length ? $('footer').outerHeight() : 0;
			if($('body').hasClass('transparent-header-footer')){
				var ch = $('#wpadminbar').length ? wh - 32 : wh;	
			}else if($('body').hasClass('transparent-header')){
				var ch = $('#wpadminbar').length ? wh - (fh + 32) : wh - fh;
			}else if($('body').hasClass('transparent-footer')){
				var ch = $('#wpadminbar').length ? wh - (hh + 32) : wh - hh;
			}else{
				var ch = $('#wpadminbar').length ? wh - (hh + fh + 32) : wh - (hh + fh);
			}
			if($('body').hasClass('framed-body')){
				var borderWidth = parseInt($('body').css('border-left-width'));
				ch -= borderWidth * 2;
			}
			$this.height(ch)
		}
	}
	$(window).on('resize', setFitScreenHeight);
	$(document).on('ready', setFitScreenHeight);
})(jQuery);

/*
*	fit-to-screen min-height
*/
(function($){
	"use strict";
	var setFitScreenMinHeight = function(){
		if($(".fit-to-screen-mh").length && ($(window).width() > 767)){
			var $this = $(".fit-to-screen-mh");
			var wh = $(window).height();
			var hh = $('header').outerHeight();
			var fh = $('footer').length ? $('footer').outerHeight() : 0;
			if($('body').hasClass('transparent-header-footer')){
				var ch = $('#wpadminbar').length ? wh - 32 : wh;	
			}else if($('body').hasClass('transparent-header')){
				var ch = $('#wpadminbar').length ? wh - (fh + 32) : wh - fh;
			}else if($('body').hasClass('transparent-footer')){
				var ch = $('#wpadminbar').length ? wh - (hh + 32) : wh - hh;
			}else{
				var ch = $('#wpadminbar').length ? wh - (hh + fh + 32) : wh - (hh + fh);
			}
			$this.css('min-height', ch);
		}
	}
	$(window).on('resize', setFitScreenMinHeight);
	$(document).on('ready', setFitScreenMinHeight);
})(jQuery);

(function($){
	"use strict";
	/*
	*	check if Royal Slider is initialized
	*	loop through items to initialize each
	*/
	if($('[data-royal-slider]')){
		$.each($('[data-royal-slider]'), function(){
			var $this = $(this);
			var options = $this.data('royal-slider-options');
			var currentVariations = undefined;
			
			$(document).on('ready', function(){
				if(options.length != 0){

					$this.royalSlider(options);

					var slider = $this.data('royalSlider');

					$(window).on('load', function(){

						slider.playVideo();

					});

					$this.parents('.royal-slider-container').find('.prev').on('click', function(){
						slider.prev();  // prev slide
					});

					$this.parents('.royal-slider-container').find('.next').on('click', function(){
						slider.next();  // next slide
					});

					slider.ev.on('rsAfterSlideChange', function(event) {
						slider.playVideo();
						var vw = $(window).width();
						var vh = (9 * vw) / 16;
						var prop = $(window).height() / vh;
						if(prop > 1){
							$('.rsVideoFrameHolder > iframe').css('transform', 'scale(' + prop + ')');
						}

						if(typeof currentVariations !== "undefined"){
							currentVariations.stopTyping();
							$('.word-variations').html('');
						}

						var instances = slider.currSlide.content.find('.word-variations');

						currentVariations = undefined;

						setTimeout(function(){
							$.each(instances, function(){
								currentVariations = new wordVariation(this);
							});
						}, 600);	
					});

					$(window).on('load', function(){
						var instances = slider.currSlide.content.find('.word-variations');
						setTimeout(function(){
							$.each(instances, function(){
								currentVariations = new wordVariation(this);
							});
						}, 800);
					});

					$(window).on('resize load', function(){
						if(1 >= slider.numSlides){
							$this.parents('.royal-slider-container').find('nav').hide();
						}else{
							$this.parents('.royal-slider-container').find('nav').show();
						}
					});

					// slider.ev.on('rsVideoPlay', function() {
					//     setTimeout(function(){
					//     	slider.next();
					//     },3000)
					// });

				}else{
					$this.royalSlider({
						keyboardNavEnabled: true,
						imageScaleMode: 'fill',
						controlNavigation: 'none',
						transitionType: 'fade'
					});  
				}
			});
		});
	}
})(jQuery);

/*
*	check if owl is initialized
*	loop through items to initialize each
*/
(function($){
	"use strict";
if($('[data-owl-carousel]')){
	$.each($('[data-owl-carousel]'), function(){
		var $this = $(this);
		var options = $this.data('owl-carousel-options');
		var navigation = $this.data('navigation');
		var activeClass = $this.data('owl-active-item');
		
		// document ready changed to window load because of autoHeight
		$(window).on('load', function(){
			if(options.length != 0){
				$this.on('initialized.owl.carousel', function(event){
					if(event.page.size >= event.item.count){
						$this.parents('.owl-container').find('nav').hide();
					}else{
						$this.parents('.owl-container').find('nav').show();
					}

				});
				var instance = $this.owlCarousel(options);

				var slider = $this;

				$this.parents('.owl-container').find('nav').find('.prev').on('click', function(){
					slider.trigger('prev.owl.carousel', [300]);
				});

				$this.parents('.owl-container').find('nav').find('.next').on('click', function(){
					slider.trigger('next.owl.carousel', [300]);
				});

				slider.on('resized.owl.carousel', function(event){
					if(event.page.size >= event.item.count){
						$this.parents('.owl-container').find('nav').hide();
					}else{
						$this.parents('.owl-container').find('nav').show();
					}

				});

			}else{
				$this.owlCarousel({
					nav : true,
					slideSpeed : 300,
					paginationSpeed : 400,
					items : true,
					transitionStyle: "fade"
				});
			}
		});
	});
}
})(jQuery);


/*
*	initialize masonry layout for home page
*/
+function($){
	"use strict";
	var $container;

	function setLayoutItemSizes(){
		var ww = $(window).width();
		$('.big-square').css({
			'width': 600,
			'height': 600
		});

		$('.small-square').css({
			'width': 300,
			'height': 300
		});

		$('.horizontal-rect').css({
			'width': 600,
			'height': 300
		});

		$('.vertical-rect').css({
			'width': 300,
			'height': 600
		});
	}

	jQuery(document).ready(function($) {
		setLayoutItemSizes();
	});

	$(window).on('load', function(){
		var smallestElm = '';
		if($('.small-rect').length){
			smallestElm = '.small-square';
		}else if($('.horizontal-rect').length){
			smallestElm = '.horizontal-rect';
		}else if($('.vertical-rect').length){
			smallestElm = '.vertical-rect';
		}else if($('.rect').length){
			smallestElm = '.big-square';
		}
		var $container = $('.horizontal-mosaic').isotope({
			layoutMode: 'masonryHorizontal',
			masonryHorizontal: {
				rowHeight: smallestElm
			}
		});
	});

}(jQuery);

/*
*	filter animation
*/
(function($){
	"use strict";
	$(window).on('load', function(){

		var filter = $(".portfolio-filter-v1").length ? $(".portfolio-filter-v1") : $(".portfolio-filter-v2");

		function setPositionOfFilter(item){
				
			var rect = item.getBoundingClientRect();

			var bodyRect = document.body.getBoundingClientRect(),
				elemRect = item.getBoundingClientRect(),
				offset   = elemRect.top - bodyRect.top,
				calcHeight = (elemRect.bottom - bodyRect.top) - (elemRect.top - bodyRect.top);

			if($('#wpadminbar').length){
				var calcTop = offset + parseInt($('html').css('margin-top'));
			} else {
				var calcTop = offset + parseInt($('html').css('margin-top'));
			}

			guxhi.animate({
				"left": rect.left,
				"top": calcTop,
				"width": rect.right - rect.left,
				"height": calcHeight,
				"position": "absolute",
				"border": "2px solid"
			});

			filter.append(guxhi);
		}

		if(filter.length){
			var	guxhi = filter.find('.guxhi'),
				firstItem = filter.find("li:first-child > a");

			guxhi.css({
				"position": "absolute",
				"border": "2px solid"
			});

			setPositionOfFilter(firstItem.get(0));

			setTimeout(function(){
				guxhi.css({
					"opacity": 1
				});
			}, 500);

			$('.filter li a').on('click', function(e){
				e.preventDefault();

				$('.filter li').removeClass('active');
				$(this).parent().addClass('active');

				setPositionOfFilter(this);

			});

			$(window).on('resize', function(){
				guxhi.fadeOut();
				var $this = $('.filter li.active a').get(0);
				setPositionOfFilter($this);
				guxhi.fadeIn();
			});
		}
	});
})(jQuery);

/*
*	mosaic
*/
(function($){
	"use strict";
	$(window).on('load', function(){
		if($(".mosaic").length){
			var $container = $('.mosaic').isotope();
		}
	});
})(jQuery);

/*
*	iconed-box
*/
(function($){
	"use strict";
	$(window).on('load', function(){
		if($(".iconed-box").length){
			var services = $(".iconed-box");
			var largestHeight = 0;
			$.each(services, function(){
				var currentServiceHeight = $(this).height();
				if(currentServiceHeight > largestHeight){
					largestHeight = currentServiceHeight;
				}
			});
			services.height(largestHeight);
		}
	});

	$(window).on('resize', function(){
		if($(".iconed-box").length){
			var services = $(".iconed-box");
			services.css('height', 'auto');
			var largestHeight = 0;
			$.each(services, function(){
				var currentServiceHeight = $(this).height();
				if(currentServiceHeight > largestHeight){
					largestHeight = currentServiceHeight;
				}
			});
			services.height(largestHeight);
		}
	});
})(jQuery);

/*
*	check if Google Map is initialized
*	loop through items to initialize each
*/
(function($){
	"use strict";
	if($('[data-map]')){
		$.each($('[data-map]'), function(){
			var $this = $(this);
			var options = $this.data('data-map-options');
			var singularity = 0;
			var myLatLng;
			
			$(document).on('ready', function(){
				var element = $this.get(0);

				var options = $(element).data('mapOptions');

				var otherMarkers = $this.find('.marker');

				if($(element).data('mapstyle').length){
					var mapStyle = [{"featureType":"all","elementType":"labels.text.fill","stylers":[{"saturation":36},{"color":"#000000"},{"lightness":40}]},{"featureType":"all","elementType":"labels.text.stroke","stylers":[{"visibility":"on"},{"color":"#000000"},{"lightness":16}]},{"featureType":"all","elementType":"labels.icon","stylers":[{"visibility":"off"}]},{"featureType":"administrative","elementType":"geometry.fill","stylers":[{"color":"#000000"},{"lightness":20}]},{"featureType":"administrative","elementType":"geometry.stroke","stylers":[{"color":"#000000"},{"lightness":17},{"weight":1.2}]},{"featureType":"landscape","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":20}]},{"featureType":"poi","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":21}]},{"featureType":"road.highway","elementType":"geometry.fill","stylers":[{"color":"#000000"},{"lightness":17}]},{"featureType":"road.highway","elementType":"geometry.stroke","stylers":[{"color":"#000000"},{"lightness":29},{"weight":0.2}]},{"featureType":"road.arterial","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":18}]},{"featureType":"road.local","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":16}]},{"featureType":"transit","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":19}]},{"featureType":"water","elementType":"geometry","stylers":[{"color":"#000000"},{"lightness":17}]}]
				} else {
					var mapStyle = [
						{"featureType": "landscape","stylers": [{"saturation": -100},{"lightness": 65},{"visibility": "on"}]},
						{"featureType": "poi","stylers": [{"saturation": -100},{"lightness": 51},{"visibility": "simplified"}]},
						{"featureType": "road.highway","stylers": [{"saturation": -100},{"visibility": "simplified"}]},
						{"featureType": "road.arterial","stylers": [{"saturation": -100},{"lightness": 30},{"visibility": "on"}]},
						{"featureType": "road.local","stylers": [{"saturation": -100},{"lightness": 40},{"visibility": "on"}]},
						{"featureType": "transit","stylers": [{"saturation": -100},{"visibility": "simplified"}]},
						{"featureType": "administrative.province","stylers": [{"visibility": "off"}]},
						{"featureType": "water","elementType": "labels","stylers": [{"visibility": "on"},{"lightness": -25},{"saturation": -100}]},
						{"featureType": "water","elementType": "geometry","stylers": [{"hue": "#ffff00"},{"lightness": -25},{"saturation": -97}]}
					]
				}

				var mapOptions = {
					zoom: options.zoom,
					disableDefaultUI: true,
					draggable: false,
					scrollwheel: false,
					styles: mapStyle
				};

				var map = new google.maps.Map(element, mapOptions);

				if(otherMarkers.length){

					if(otherMarkers.length == 1){
						$.each(otherMarkers, function(){
							var myLatLng2 = new google.maps.LatLng($(this).data('lat'), $(this).data('lng'));
							myLatLng = myLatLng2;
							var marker = new google.maps.Marker({
								position: myLatLng2,
								map: map,
								icon: options.icon,
								title: options.title
							});
							map.setCenter(myLatLng2);
						});
					}else{
						var bounds = new google.maps.LatLngBounds();

						$.each(otherMarkers, function(){
							var myLatLng2 = new google.maps.LatLng($(this).data('lat'), $(this).data('lng'));
							bounds.extend( myLatLng2 );
							var marker = new google.maps.Marker({
								position: myLatLng2,
								map: map,
								icon: options.icon,
								title: options.title
							});
						});

						map.fitBounds( bounds );
					}

				}

				var marker = new google.maps.Marker({
					position: myLatLng,
					map: map,
					icon: options.icon,
					title: options.title
				});

				var centerOfCenter;

				if($('.map-contact-info').length && $(window).width() >= 768 ){
					google.maps.event.addListener(map, 'bounds_changed', function() {
						if(!singularity){
							var mapNE = map.getBounds().getSouthWest();
			         		map.setCenter(new google.maps.LatLng(myLatLng.lat(), (mapNE.lng() + myLatLng.lng()) / 2));
			         		singularity++;
						}
			      	});
				}
			});
		});
	}
})(jQuery);

/*
*	vertical-projects
*/
(function($){
	"use strict";
	$(document).on('ready', function(){
		if($('.vertical-projects').length){
			if($('.vertical-projects').parents('.autoslide').length){
				return false;
			}
			var projectsContainer = $('.vertical-projects');
			var projects = projectsContainer.find('.vertical-project');
			var numberOfProjects = projects.length;

			projects.width(100 / numberOfProjects + "%");

			projects.hover(
				function(){
					var containerSize = projectsContainer.width();

					var toExpand = $(this).find('img').width();
					if(toExpand > $(window).width() - ((numberOfProjects - 1) * 230)){
						toExpand = $(window).width() - ((numberOfProjects - 1) * 230);
					}
					//
					// containerSize:toExpand = 100:x
					var toExpandPerc = (toExpand*100) / containerSize;
					console.log(toExpandPerc)

					var leftSize = containerSize - toExpand;
					var sizeOfLeftElements = leftSize / (numberOfProjects - 1);
					//
					// containerSize:sizeOfLeftElements = 100:x
					var sizeOfLeftElementsPerc = (sizeOfLeftElements*100) / containerSize;
					console.log(sizeOfLeftElementsPerc)


					projects.not($(this)).css({
						'width': sizeOfLeftElementsPerc + "%"
					});
					$(this).css({
						'width': toExpandPerc + "%"
					});
				},
				function(){
					projects.css({
						'width': 100 / numberOfProjects + "%"
					});
				}
			);
		}
	});
})(jQuery);

/*
*	vertical-projects
*	testing autoslide
*/
(function($){
	// "use strict";
	$(document).on('ready', function(){
		if($('.autoslide .vertical-projects').length){
			var projectsContainer = $('.vertical-projects');
			var projects = projectsContainer.find('.vertical-project');
			var numberOfProjects = projects.length;
			var currentItemIndex = 1;
			var autoslideTimer = null;

			function calculateSizeToExpand($this){
				var containerSize = projectsContainer.width();

				var toExpand = $this.find('img').width();
				if(toExpand > $(window).width() - ((numberOfProjects - 1) * 230))
					toExpand = $(window).width() - ((numberOfProjects - 1) * 230);

				return toExpand
			}

			function calculateSizeOfElements($this){
				var containerSize = projectsContainer.width();

				var toExpand = calculateSizeToExpand($this)

				var leftSize = containerSize - toExpand;
				var sizeOfLeftElements = leftSize / (numberOfProjects - 1);

				return sizeOfLeftElements
			}

			function autoSlide(){
				autoslideTimer = setInterval(function(){

					var currentItem = $(projects[currentItemIndex % numberOfProjects])

					projects.css({
						'width': 100 / numberOfProjects + "%"
					});

					var sizeOfElements = calculateSizeOfElements(currentItem)
					var sizeToExpand = calculateSizeToExpand(currentItem)

					projects.not(currentItem).css({
						'width': sizeOfElements
					});
					currentItem.css({
						'width': sizeToExpand
					});

					currentItemIndex++;
					currentItemIndex = currentItemIndex % numberOfProjects;

				}, 2000)

			}

			projects.width(100 / numberOfProjects + "%");

			projects.hover(
				function(){
					projects.not($(this)).css({
						'width': calculateSizeOfElements($(this))
					});
					$(this).css({
						'width': calculateSizeToExpand($(this))
					});
					currentItemIndex = (projects.index($(this)) + 1) % numberOfProjects
					clearInterval(autoslideTimer)
				},
				function(){
					projects.css({
						'width': 100 / numberOfProjects + "%"
					});
					autoSlide();
				}
			);

			projects.not($(projects[0])).css({
				'width': calculateSizeOfElements($(projects[0]))
			});
			$(projects[0]).css({
				'width': calculateSizeToExpand($(projects[0]))
			});

			autoSlide();
		}
	});
})(jQuery);

// lazy load
(function($){
	"use strict";
	$("img.lazy").lazyload({
		effect : "fadeIn",
		placeholder: ""

	});
})(jQuery);


/*
*	twitter-widget
*/
(function($){
	"use strict";
	$(document).on('ready', function(){
		$.each($('[data-twitter-widget]'), function(){
			var self = $(this),
				delay = self.data('twitter-widget-options').delay,
				random = self.data('twitter-widget-options').random,
				tweets = self.find('li');
				currentTweetIndex = 0;

			tweets.hide();

			if(random){
				var currentTweetIndex = Math.floor((Math.random() * tweets.length));
				tweets.eq(currentTweetIndex).show();
				setInterval(function(){
					tweets.eq(currentTweetIndex).hide();
					var oldTweetIdex = currentTweetIndex;
					currentTweetIndex = Math.floor((Math.random() * tweets.length));
					tweets.eq(currentTweetIndex).fadeIn();
				},delay);
			}else{
				tweets.eq(currentTweetIndex).show();
				setInterval(function(){
					tweets.eq(currentTweetIndex).hide();
					currentTweetIndex++;
					currentTweetIndex = currentTweetIndex % tweets.length;
					tweets.eq(currentTweetIndex).fadeIn();
				}, delay)
			}
		});
	});
})(jQuery);

/*
* Blog Ajax
*/
(function($){
	"use strict";
	var container = $('#blog-items');
	container.siblings('.load-more-container').find(".loadMoreBtn").on('click', function(e){
    e.preventDefault();
   
    var loadBtnContainer = $(this);
    loadBtnContainer.addClass('loading');
    blog_page++;
    
    var ajax_data = {
    	"action": "stonedthemes_blog",
        "blog_page" : blog_page,
        "post_categories":post_categories,
        "grid_elements":grid_elements,
        "orderby":orderby,
        "sorting":sorting,
    };

    if (typeof blog_masonry !== 'undefined') {
    	ajax_data = {
    	"action": "stonedthemes_blog_masonry",
        "blog_page" : blog_page,
        "post_categories":post_categories,
        "grid_elements":grid_elements,
        "orderby":orderby,
        "sorting":sorting,
   		 };
    }
        $.post(stonedthemes_Ajax.ajaxurl,ajax_data,function(data){
           if(data){
           		if (typeof blog_masonry !== 'undefined') {
           			$(data).imagesLoaded( function(){	
	                	$(container).isotope( 'insert', $(data));
						$(document).trigger('itemsChanged');
						loadBtnContainer.removeClass('loading');			
	                });
           		} else {
           			$(data).imagesLoaded( function(){					
						$(container).append($(data));
                	});
           		}

              
                if(blog_last_page == blog_page ){
                    $("a.loadMoreBtn").fadeOut();
                }
                
                loadBtnContainer.removeClass('loading');
            }
		
        }).error(function(){
			container.siblings('.load-more-container').find(".loadMoreBtn").fadeOut();
        });
        
    });
})(jQuery);

/*
* Portfolio Ajax
*/
(function($){
	"use strict";
	var container = $('#portfolio-itmes');

	container.siblings('.load-more-container').find(".loadMoreBtn").on('click', function(e){
        e.preventDefault();
        if (typeof portfolioMosaic !== 'undefined') {
       		loadMore(container,"stonedthemes_portfolio_mosaic_load_more");
        } else {
			loadMore(container,"stonedthemes_portfolio_load_more");
		}	
    });

 	$('body').on('click', '.filter li > a', function(e){			
		e.preventDefault();
		if (typeof portfolioMosaic !== 'undefined') {

		 	var filterValue = $(this).attr('data-filter');
			$(".mosaic").isotope({ filter: filterValue });

			setTimeout(function(){
				$("html,body").trigger("scroll");
			}, 500)

		} else {	
			categorization($(this).data("slug"),container,"stonedthemes_portfolio_load_more");
			container.siblings('.load-more-container').find("a.loadMoreBtn").fadeOut();		
		}
		
	});	
})(jQuery);	

function loadMore(container,portfolio){
	"use strict";
	var $ = jQuery;
	var loadBtnContainer = $('.loadMoreBtn');
	loadBtnContainer.addClass('loading');
	
    page++;
    var ajax_data = {
    	"action" : portfolio,
        "paged" : page,
        "postID" : $(container).data("id")
    };

    $.post(stonedthemes_Ajax.ajaxurl,ajax_data,function(data){
        if(data){

            $(data).imagesLoaded( function(){

           		var dataToDisplay = $(data);
            	if(portfolio == "stonedthemes_portfolio_mosaic_load_more"){
            		var $grid = container;
            		// dataToDisplay = dataToDisplay.filter(".portfolio-item-container");
            		// console.log(data);
            		  $grid.append( dataToDisplay ).isotope( 'appended', dataToDisplay );
            	}else{
            		dataToDisplay.css('opacity', 0);
            		$(container).append(dataToDisplay);
	            	var itemCounter = 0;
	            	$.each(dataToDisplay, function(i){
	            		var $this = $(this);
	            		if($this.hasClass('portfolio-item-container')){
	            			itemCounter++;
	            			setTimeout(function(){
		            			$this.animate({
		            				'opacity': 1
		            			})
			            	}, itemCounter * 100);
	            		}
	            	});
            	}
				
	            if(last_page == page ){
	                loadBtnContainer.fadeOut();
	            }
	            
		        loadBtnContainer.removeClass('loading');
            });
        }
	
    }).error(function(){
            loadBtnContainer.find("a.loadMoreBtn").fadeOut();
    });
}

function categorization(category,container,portfolio){
	"use strict";
	var $ = jQuery;

    var ajax_data = {
    	"action" : portfolio,
        "category" : category,
        "postID" : $(container).data("id")
    };  

    // fade out current elms
    var elements = $('.portfolio-item');
    elements.fadeOut(function(){
    	$(this).remove();
    });

    setTimeout(function(){
    	container.siblings('.portfolio-loading').fadeIn();
    }, 600);

    $.post(stonedthemes_Ajax.ajaxurl,ajax_data,function(data){
        if(data){
            $(data).imagesLoaded( function(){
            	var dataToDisplay = $(data);
            	dataToDisplay.css('opacity', 0);
				$(container).html(dataToDisplay);
            	var itemCounter = 0;
            	$.each(dataToDisplay, function(i){
            		var $this = $(this);
            		if($this.hasClass('portfolio-item-container')){
            			itemCounter++;
            			setTimeout(function(){
	            			$this.animate({
	            				'opacity': 1
	            			})
		            	}, itemCounter * 100);
            		}
            	});
            	container.siblings('.portfolio-loading').hide()
            });
            if(last_page == page ){
                container.siblings('.load-more-container').find("a.loadMoreBtn").fadeOut();
            }
        }
	
    }).done(function(){
    	// console.log('test');
    })
    .error(function(){
		jQuery("a.loadMoreBtn").fadeOut();
    });
}


/*
* Lightbox Ajax
*/
(function($){
	"use strict";
	
 	$('body').on('click', '.ajax_lightbox', function(e){		
 	
 		e.preventDefault();	
		var ajax_data = {
	    	"action" : "stonedthemes_lightbox",
	        "postID" : $(this).data("id")
	    };

	    var postTitle = $(this).data('title');
	    var postCategory = $(this).data('cat');

	    var modalStructure = '<div class="modal fade" id="sth-lightbox" tabindex="-1" role="dialog" aria-labelledby="myModalLabel">' +
								'<div class="modal-dialog" role="document">' +
									'<div class="modal-content">' +
    									'<div class="modal-body">' +
    										'<div class="modal-carousel-header text-center">' +
												'<div class="slider-number">' +
													'<div class="slide-positions">' +
														'<span class="current-slide">01</span>' +
														'<span class="all-slides">15</span>' +
													'</div>' +
												'</div>' +
												'<div class="slide-title">' +
    												'<h4 class="text-uppercase"></h4>' +
    												'<p></p>' +
												'</div>' +
												'<div class="close-modal">' +
													'<div class="close-lightbox"></div>' +
												'</div>' +
        									'</div>' +
            								'<div class="modal-carousel">' +
            									'<div class="vertical-center2-container">' +
            										'<div class="vertical-center2-content text-center"></div>' +
            									'</div>' +
            									'<div class="owl-container">' +
                    								'<div class="owl-carousel"></div>' +
                    								'<nav class="side style-1">' +
														'<div class="prev">' +
															'<i class="fa fa-angle-left fa-vertical-center"></i>' +
														'</div>' +
														'<div class="next">' +
															'<i class="fa fa-angle-right fa-vertical-center"></i>' +
														'</div>' +
													'</nav>' +
												'</div>' +
            								'</div>' +
            								'<div class="modal-carousel-footer">' +
        									'</div>' +
        								'</div>' +
    								'</div>' +
								'</div>' +
							'</div>';

		var modal = $("#sth-lightbox");
		if(!modal.length){
			modal = $(modalStructure);
			$('body').append(modal);
		}

		modal.find('.slide-title h4').text(postTitle);
		modal.find('.slide-title p').text(postCategory);

		modal.find('.vertical-center2-container').show();
		modal.modal('show');
	    
	    $.post(stonedthemes_Ajax.ajaxurl,ajax_data,function(data){
       		if(data){
            	var imageUrls = JSON.parse(data);
            	// console.log(imageUrls);

				// var galleryId = $(this).attr("rel");
				var imagesOnGallery = imageUrls;
				var numToDisplay = imagesOnGallery.length < 10 ? "0" + imagesOnGallery.length : imagesOnGallery.length
				modal.find('.all-slides').html(numToDisplay);

				var clickedImageIndex = 0;
				var clickedImageIndexToDisplay = clickedImageIndex < 10 ? "0" + (clickedImageIndex + 1) : (clickedImageIndex + 1)
				modal.find('.current-slide').html(clickedImageIndexToDisplay);

				var modalCarouselContainer = modal.find(".owl-carousel");
				modalCarouselContainer.html('');
				modalCarouselContainer.css('opacity', 0);

				for (var i = 0; i < imagesOnGallery.length; i++) {
					var galleryItemMarkup = $('<img src="'+imagesOnGallery[i]+'" />');
					// console.log(galleryItemMarkup);
					modalCarouselContainer.append(galleryItemMarkup);
				};

				// $.each(imagesOnGallery, function(){
				// 	console.log($(this))
				// 	var galleryItemMarkup = $('<img src="'+$(this).data('original')+'" />');
				// 	modalCarouselContainer.append(galleryItemMarkup);
				// });
				
				$(modalCarouselContainer).imagesLoaded( function(){	
		        	setTimeout(function(){
		        		modal.find('.vertical-center2-container').hide();

		        		modalCarouselContainer.fadeIn();

						var imagesToSetWidth = modalCarouselContainer.find('img');
						$.each(imagesToSetWidth, function(){
							var width = $(this).width();
							$(this).width(width);
						})

						var slider = modal.find('.owl-carousel').owlCarousel({
							"margin": 30,
							"loop": true,
							"autoWidth":true,
							"center": true,
							"startPosition": clickedImageIndex
						});

						slider.parents('.owl-container').find('nav').find('.prev').on('click', function(){
							slider.trigger('prev.owl.carousel', [300]);
						});

						slider.parents('.owl-container').find('nav').find('.next').on('click', function(){
							slider.trigger('next.owl.carousel', [300]);
						});

						$(document).on('keydown', function(e){
						    var type = e.which == 39? 'next': null,
						        type = e.which == 37? 'prev': type;
					        slider.trigger(type+'.owl.carousel');
						});

						slider.on('changed.owl.carousel', function(event){
							var currentItemIndex = event.page.index < 10 ? "0" + (event.page.index + 1) : (event.page.index + 1)
							modal.find('.current-slide').html(currentItemIndex);
						});

			        	modalCarouselContainer.css('opacity', 1);

						$('.close-lightbox').on('click', function(){
							modal.modal('hide');
						});

						modal.on("hidden.bs.modal", function(){
							slider.trigger("destroy.owl.carousel");
							$(document).off('keydown');
							slider.parents('.owl-container').find('nav').find('.prev').off('click');
							slider.parents('.owl-container').find('nav').find('.next').off('click');
							modalCarouselContainer.html('')
						});
		        	},1000);
		        });
            }
        }).done(function(){
	    	// console.log('test');
	    })
	    .error(function(){
			
	    });
        
	});	
})(jQuery);	


/*
*	vertical-projects
*/
(function($){
	"use strict";
	$(window).on('load', function(){
		if($('.blog-masonry').length){
			var $container = $('.blog-masonry').isotope({
				layoutMode: 'masonry',
				itemSelector: '.masonry-item'
			});
		}
	});
})(jQuery);


(function($){
	"use strict";
	$('.header-v5 .menu-icon > a').on('click', function(e){
		e.preventDefault();
		$(this).toggleClass('active-header-item');
		$(this).parent().children('div').fadeToggle();
		$(this).find('i').toggleClass('fa-navicon');
		$(this).find('i').toggleClass('fa-times');

		if($(this).find('i').hasClass('fa-navicon')){
			var menuItems = $(this).parent().children('div').find(' > ul > li');
			menuItems.css({'opacity':0});
		}else{
			var menuItems = $(this).parent().children('div').find(' > ul > li');
			$.each(menuItems, function(i, el){
				$(el).css({'opacity':0});
				setTimeout(function(){
					$(el).animate({
					'opacity': 1
					}, 400);
				},400 + ( i * 120 ));
			});
		}
	});

	$(document).on('keyup', function(e){
		var $this = $('.header-v5 .menu-icon > a');
		if(e.keyCode == 27){
			if ($this.find('i').hasClass('fa-times')) {
				// console.log(e.keyCode);
				$this.toggleClass('active-header-item');
				$this.parent().children('div').fadeToggle();
				$this.find('i').toggleClass('fa-navicon');
				$this.find('i').toggleClass('fa-times');
				var menuItems = $this.parent().children('div').find(' > ul > li');
				menuItems.css({'opacity':0});
			};
		}
	})

	$(".header-v5 ul.menu li > a").on('click', function(w){
		
		if($(this).attr('href') == 0 || $(this).attr('href') == "#"){

			w.preventDefault();

			if($(this).parent().hasClass('active')){
				$(this).parent().toggleClass('active');
				$(this).parent().children("ul.sub-menu").stop().slideToggle();
				// console.log("case 1");
			}

			else if($(this).parents('li').hasClass('active')){
				$(this).parent().addClass('active');
				$(this).parent().children("ul.sub-menu").stop().slideDown();
				// console.log("case 2");
			}

			else{
				$(".header-v5 ul.menu li").removeClass('active');
				$("ul.sub-menu").slideUp();
				$(this).parent().addClass('active');
				$(this).parent().children("ul.sub-menu").stop().slideDown();
				// console.log("case 3");
			}

		}

	});
})(jQuery);

/*
*	check if CustomScrollbar is initialized
*	loop through items to initialize each
*/
(function($){
	"use strict";
	if($('[data-customScrollbar]')){
		var isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;
		if(!isMac){
			$.each($('[data-customScrollbar]'), function(){
				var $this = $(this);
				var options = $this.data('customscrollbaroptions')
				// console.log(options)
				$(window).on('load', function(){
					$this.mCustomScrollbar(options);
				});			
			});
		}
	}
})(jQuery);

/*
*	responsive menu toggle
*/
(function($){
	"use strict";
	$(document).on('click', 'a.responsive-menu-btn', function(w){
		w.preventDefault();
		$(this).find('i').toggleClass('fa-times');
		$(this).find('i').toggleClass('fa-bars');
		$('.responsive-menu ul.menu').slideToggle();
	});
})(jQuery);

/*
*	word variations initialization
*/
(function($){
	"use strict";
	$(window).on('load', function(){
		setTimeout(function(){
			var instances = $('.word-variations');
			$.each(instances, function(){
				if(!($(this).parents('.royal-slider').length)){
					new wordVariation(this);
				}
			});
		},2000);
	});
})(jQuery);

/*
*	single project lightbox
*/
(function($){
	"use strict";
	$('.portfolio-images a').on("click", function(e){
		e.preventDefault();

		var modal = $("#sth-lightbox");
		modal.find('.vertical-center2-container').show();
		modal.modal('show');

		var galleryId = $(this).attr("rel");
		var imagesOnGallery = $(".portfolio-images").find("span img:first-of-type");
		var numToDisplay = imagesOnGallery.length < 10 ? "0" + imagesOnGallery.length : imagesOnGallery.length
		modal.find('.all-slides').html(numToDisplay);

		var clickedImageIndex = $(".portfolio-images a").index(this);
		var clickedImageIndexToDisplay = clickedImageIndex < 10 ? "0" + (clickedImageIndex + 1) : (clickedImageIndex + 1)
		modal.find('.current-slide').html(clickedImageIndexToDisplay);

		var modalCarouselContainer = modal.find(".owl-carousel");
		modalCarouselContainer.html('');
		modalCarouselContainer.css('opacity', 0);

		$.each(imagesOnGallery, function(){
			var galleryItemMarkup = $('<img src="'+$(this).data('original')+'" />');
			modalCarouselContainer.append(galleryItemMarkup);
		});
		
		$(modalCarouselContainer).imagesLoaded( function(){	
        	setTimeout(function(){
        		modal.find('.vertical-center2-container').hide();

        		modalCarouselContainer.fadeIn();

				var imagesToSetWidth = modalCarouselContainer.find('img');
				$.each(imagesToSetWidth, function(){
					var width = $(this).width();
					$(this).width(width);
				})

				var slider = $('.owl-carousel').owlCarousel({
					"margin": 30,
					"loop": true,
					"autoWidth":true,
					"center": true,
					"startPosition": clickedImageIndex
				});

				slider.parents('.owl-container').find('nav').find('.prev').on('click', function(){
					slider.trigger('prev.owl.carousel', [300]);
				});

				slider.parents('.owl-container').find('nav').find('.next').on('click', function(){
					slider.trigger('next.owl.carousel', [300]);
				});

				$(document).on('keydown', function(e){
				    var type = e.which == 39? 'next': null,
				        type = e.which == 37? 'prev': type;
			        slider.trigger(type+'.owl.carousel');
				});

				slider.on('changed.owl.carousel', function(event){
					var currentItemIndex = event.page.index < 10 ? "0" + (event.page.index + 1) : (event.page.index + 1)
					modal.find('.current-slide').html(currentItemIndex);
				});

	        	modalCarouselContainer.css('opacity', 1);

				$('.close-lightbox').on('click', function(){
					modal.modal('hide');
				});

				modal.on("hidden.bs.modal", function(){
					slider.trigger("destroy.owl.carousel");
					$(document).off('keydown');
					slider.parents('.owl-container').find('nav').find('.prev').off('click');
					slider.parents('.owl-container').find('nav').find('.next').off('click');
				});
        	},1000);
        });
	});
})(jQuery);

/*
*	word variations initialization
*/
(function($){
	"use strict";
	var goToTop = $('#to-top');
	$(window).on('load scroll', function(){
		var top = $( document ).scrollTop();
		if(top >= 500){
			goToTop.fadeIn();
		}else{
			goToTop.hide();
		}
	});
	goToTop.on('click', function(){
		$("html, body").animate({
			scrollTop: 0
		});
	});
})(jQuery);

/*
*	scaleVideo
*/
(function($){
	"use strict";
	$(window).on('load resize', function(){
		var vw = $(window).width();
		var vh = (9 * vw) / 16;
		var prop = $(window).height() / vh;
		if(prop > 1){
			setTimeout(function(){
				$('.rsVideoFrameHolder > iframe').css('transform', 'scale(' + prop + ')');
			}, 400)
		}
	});
})(jQuery);


/*
*	responsive menu
*/
(function($){
	"use strict";
	$(document).on('ready', function(){
		$('.current-menu-ancestor').addClass('active');
		$('.current-menu-parent').addClass('active');

		$(".responsive-menu ul.menu li > a").on('click', function(w){
		
			if($(this).attr('href') == 0 || $(this).attr('href') == "#"){

				w.preventDefault();

				if($(this).parent().hasClass('active')){
					$(this).parent().toggleClass('active');
					$(this).parent().children("ul.sub-menu").stop().slideToggle();
				}

				else if($(this).parents('li').hasClass('active')){
					$(this).parent().addClass('active');
					$(this).parent().children("ul.sub-menu").stop().slideDown();
				}

				else{
					$(".header-v5 ul.menu li").removeClass('active');
					$("ul.sub-menu").slideUp();
					$(this).parent().addClass('active');
					$(this).parent().children("ul.sub-menu").stop().slideDown();
				}
			}
		});

	});
})(jQuery);

/*
*	header display/hide shares
*/
(function($){
	"use strict";
	$('.shares > a').on('click', function(e){
		var $this = $(this);
		if($(this).siblings('ul').length){
			e.preventDefault();
			$this.toggleClass('active')
			var items = $(this).parent().find('ul li');
			if(items.css('display') == 'list-item'){
				items = $(this).parent().find('ul li').get().reverse();
			}
			$.each(items, function(i){
				var item = $(this);
				setTimeout(function(){
					item.stop().fadeToggle(400);
				}, i * 200);
			});
		}

	});
})(jQuery);

/*
*	hide loading screen on animation end
*/
(function(){
	"use strict";
	var el = document.getElementById("loading-container");

	function hide(){
		el.style.display = "none";
	}

	el.addEventListener("webkitAnimationEnd", hide);

	el.addEventListener("animationend", hide);
})();


/*
*	adjust modal display
*/
(function($){
	"use strict";
	var modal = $('.modal');
	modal.on('hide.bs.modal', function(){
		if(!($(this).hasClass('out'))){
			$(this).addClass('out');
		}
	});
	modal.on('hidden.bs.modal', function(){
		if($(this).hasClass('out')){
			$(this).removeClass('out');
		}
	});
})(jQuery);

/*
*	single product gallery
*/
(function($){
	"use strict";
	var $this = $('.single-product .product .images');
	// console.log($this.find('img').length)
	if($this.find('img').length > 1){
		var thumbsCnt = $('<div></div>').addClass('thumbs-container');
		thumbsCnt.insertAfter($this);
		var imagesForThumb = $this.find('img');
		var imgSrcs = [];
		var r1 = /\d{3}/g;
		var r2 = new RegExp(r1.source + 'x' + r1.source);

		$.each(imagesForThumb, function(i){
			var thumbImgSrc = $(this).attr('src');
			var thumbSrcReplace = thumbImgSrc.replace(r2, '180x180');
			if(i == 0){
				thumbsCnt.append($('<img>').attr('src', thumbSrcReplace).addClass('active'));
			}else{
				thumbsCnt.append($('<img>').attr('src', thumbSrcReplace));
			}
		});

		$this.addClass('owl-carousel');
		thumbsCnt.addClass('owl-carousel');

		$(window).on('load', function(){
			$this.owlCarousel({
				items: 1,
				nav: true,
				autoHeight: true,
				navText: ['<i class="fa fa-chevron-left"></i>', '<i class="fa fa-chevron-right"></i>']
			});
			thumbsCnt.owlCarousel({
				items: 6,
				margin: 1,
				nav: false
			});
		});

		thumbsCnt.find('img').on('click', function(){
			var images = thumbsCnt.find('img')
			var clickedImageIndex = images.index(this);
			if(!($(this).hasClass('active'))){
				images.removeClass('active');
				$(this).addClass('active');
			}
			$this.trigger("to.owl.carousel", [clickedImageIndex, 250, true]);
		});

		$this.on('changed.owl.carousel', function(event){
			var images = thumbsCnt.find('img');
			var currentItemIndex = event.page.index;
			if(currentItemIndex >= 0 && !(thumbsCnt.find('img').eq(currentItemIndex).hasClass('active'))){
				images.removeClass('active');
				thumbsCnt.find('img').eq(currentItemIndex).addClass('active');	
			}
		});
	}

})(jQuery);

/*
*	adjust single product summary
*/
(function($){
	"use strict";
	$(window).on('load', function(){
		var images = $('.single-product .product .images');
		var thumbs = $('.single-product .product .thumbs-container');
		var summary = $('.single-product .product .summary');
		if((images.innerHeight() + thumbs.innerHeight()) > summary.innerHeight()){
			summary.css('min-height', images.innerHeight() + thumbs.innerHeight());
		}
	});

	var refreshRate = 200;
	var calculate = true;
	$(window).on('resize', function(){
		if(calculate){
			calculate = false;
			setTimeout(function(){
				var images = $('.single-product .product .images');
				var thumbs = $('.single-product .product .thumbs-container');
				var summary = $('.single-product .product .summary');
				// if((images.innerHeight() + thumbs.innerHeight()) > summary.innerHeight()){
				summary.css('min-height', images.innerHeight() + thumbs.innerHeight());
				// }
				calculate = true;
			}, refreshRate);
		}
	});
})(jQuery);

/*
*	scrolable image
*/
(function($){
	$('.scrollable-image .content').on({
		mouseover: function(){
			var $this = $(this);
			var h = $this.height();
			var i_h = $this.find('img').height();
			var toMove = h - i_h;
			var timeToMove = toMove * (-5);
			if(timeToMove < 400){
				timeToMove = 400;
			}
			if(toMove < 0) {
				$(this).find('img').stop().animate({'top': toMove}, timeToMove);
			}
		},
		mouseleave: function(){
			$(this).find('img').stop().animate({top:'0'}, 'fast');
		}
	});
})(jQuery);

(function($){
	function isOnScreen(element){
	    
	    var win = $(window);
	    
	    var viewport = {
	        top : win.scrollTop(),
	        left : win.scrollLeft()
	    };
	    viewport.right = viewport.left + win.width();
	    viewport.bottom = viewport.top + win.height();

	    var bounds = element.offset();
	    bounds.right = bounds.left + element.outerWidth();
	    bounds.bottom = bounds.top + element.outerHeight();
	    
	    // return (!(viewport.right < bounds.left || viewport.left > bounds.right || viewport.bottom < bounds.top || viewport.top > bounds.bottom));
	    return (!(viewport.right < bounds.left || viewport.left > bounds.right || viewport.bottom < (bounds.top + 50) || viewport.top > (bounds.bottom + 50)));
	    
	};

	$(window).on('load scroll', function(){
		var elements = $('.intro-animation').not('.animated');
		$.each(elements, function(){
			var element = $(this)
			if(isOnScreen(element)){
				element.addClass('animated');
			}
		})
	});
})(jQuery);

(function($){
 	$('.um_menuTrigger').on('click', function() {
        $(this).find('.um_menuAnimatedIcon').toggleClass('isClicked');
        $(this).find('.um_topSlideMenuText').toggleClass('isClicked');
        
        $('#pageHeader').toggleClass('um_hide');
    });
})(jQuery);

(function($){

	//Iterate to add liked items	
	$('.like').each(function(i, obj) {			
		if(readCookie('Viewed' + $(this).attr("data-id")) === $(this).attr("data-id"))
		{ 
			$(this).addClass('liked');
			// $(this).find('i').remove();
			// $(this).append('<i class="fa fa-heart fa-3x"></i>');
		}
	});

	//Project like
	$('body').on('click', '.like', function(e){	
		e.preventDefault();
		if($(this).hasClass('liked')){
			$(this).removeClass('liked');
			remove_like($(this).attr("data-id"),$(this));
			// $(this).find('i').remove();
			// $(this).append('<i class="fa fa-heart fa-2x"></i>');
		}
		else{
			insert_like($(this).attr("data-id"),$(this));
			$(this).addClass('liked');
			// $(this).find('i').remove();
			// $(this).append('<i class="fa fa-heart fa-3x"></i>');
		}
	});	
})(jQuery);

// setSizeOnGalleryItems
(function($){
	function setHeight (){
		var maxHeight = 0;
		var items = $('.variable-sized-gallery .owl-carousel .owl-item > div');
		$.each(items, function(){
			var currentHeight = $(this).height();
			if(currentHeight > maxHeight)
				maxHeight = currentHeight;
		});
		items.height(maxHeight + 1);

	}
	$(window).on('load resize', setHeight);
})(jQuery);

// one page menu
(function($){
	var isOnePage = $('body').is('#one-page.home') ? true : false;
	if(isOnePage){

		$(window).on('load scroll', stickIt);

		function stickIt() {

			var header = $('header');

			if($(window).width() < 992){
				var rm = $('.responsive-header > .responsive-menu');
				if($(window).scrollTop() >= 100){
					if(!rm.hasClass('fixed-rm')){
						rm.addClass('fixed-rm');
					}
				}else{
					if(rm.hasClass('fixed-rm')){
						rm.removeClass('fixed-rm');
					}
				}
			}else{
				if ($(window).scrollTop() > 0){
					if(!header.hasClass('fixed')){
						header.addClass('fixed');
					}
				}else{
					if(header.hasClass('fixed')){
						header.removeClass('fixed');
					}
				}	
			}
		}

		$('header .menu li a').on('click', function(e){
		    if(!($(this).attr('href').includes('http'))){
    			e.preventDefault();
    			var elToScroll = $(this).attr('href');
    			if($(window).width() < 1024){
    				var menuContainer = $('header > div.responsive-header .responsive-menu')
    				menuContainer.find('ul.menu').hide();
    				menuContainer.find('.responsive-menu-btn i').toggleClass('fa-times');
    				menuContainer.find('.responsive-menu-btn i').toggleClass('fa-bars');
    				$('html, body').scrollTop($(elToScroll).offset().top - 80)
    			}else{
    				$('html, body').animate({
    					scrollTop: $(elToScroll).offset().top - 90
    				}, 400, "swing");
    			}
		    }
		});

		$(window).on('load', function(){
			$(this).trigger('resize');
		});
	}
})(jQuery);

// blur images
(function($){

    $.fn.blurload = function(options) {
    	var elements = $(this);

		function showFullImage(image, index) {
			var $this = image;
			var parent = $this.parent();
			var bigImage = $('<img>').attr('src', $this.data('original'));
			bigImage.css('display', 'none');
			parent.append(bigImage)

			parent.imagesLoaded( function() {
				$this.css('display', 'none')
				bigImage.css('display', 'inline')
			});
		}

		function isElementInViewport (el, index) {

		    //special bonus for those using jQuery
		    if (typeof jQuery === "function" && el instanceof jQuery) {
		        el = el[0];
		    }

		    var rect = el.getBoundingClientRect();

		    return (
		    	rect.bottom >= 0 && 
		    	rect.right >= 0 && 
		    	rect.top <= (window.innerHeight || document.documentElement.clientHeight) && 
		    	rect.left <= (window.innerWidth || document.documentElement.clientWidth)
	    	);
		}

		function checkElements(){
			elements.each(function(i){
				var element = this
				var $element = $(element)
				if(isElementInViewport(element, i)){
					showFullImage($element, i);
					var temp = $.grep(elements, function(item, index) {
	                    // return !item.loaded;
	                    return $(item).attr('src') !== $element.attr('src')
	                });

	                elements = $(temp);
				}
			});
		}

		$(window).on('load scroll', function(){
			checkElements();
		});

		$(window).on('resize', function(){
			checkElements();
		})
	}

	$(document).on('ready', function(){
		var items = $("img.preload")
		items.blurload();
	})

})(jQuery);

// sticky header
(function($){
	
	if($('body').hasClass('has-sticky-header')){
    	var headerHeight = $('body > header').height();
    	var stickyMenu = $('body > header').clone().addClass('sticky-header').hide();
    	$('body').append(stickyMenu)

    	$(window).on('load scroll', function(){
			var top = $( document ).scrollTop();
			if(top >= headerHeight){
				stickyMenu.fadeIn();
			}else{
				stickyMenu.hide();
			}
		});
    }

})(jQuery);