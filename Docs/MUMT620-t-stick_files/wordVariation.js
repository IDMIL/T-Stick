function wordVariation(instance, options){
	'use strict';
	var obj = this;
	obj.options = options || {
		charAppearSpeed: 200,
		charDissapearSpeed: 100,
		afterWordCompleteTimeout: 2000,
		stop: false
	}
	obj.wordsContainer = instance;
	obj.spanElement = instance;
	obj.words = JSON.parse(obj.wordsContainer.dataset.variations).variations;
	obj.wordTimeout = 0;
	obj.timeouts = [];
	obj.stopTyping = function(){
		for (var i = 0; i < obj.timeouts.length; i++) {
			clearTimeout(obj.timeouts[i]);
		};
	}
	for(var i = 0; i < obj.words.length; i++){
		var wordsChars = obj.words[i].split('');
		var wordIndex = 0;
		if(i > 0){
			obj.wordTimeout += (obj.words[i-1].length * obj.options.charAppearSpeed) + obj.options.afterWordCompleteTimeout;
			obj.wordTimeout += obj.words[i-1].length * obj.options.charDissapearSpeed;
		}
		var wordTimeout = setTimeout(function(){
			var currentWord = obj.words[wordIndex];
			var currentWordChars = currentWord.split('');
			for(var j = 0; j < currentWordChars.length; j++){
				var currentCharIndex = 0;
				obj.spanElement.innerHTML = '';
				var appearingTimeout = setTimeout(function(){
					obj.spanElement.innerHTML += currentWordChars[currentCharIndex];
					currentCharIndex++;
					if(obj.options.stop){
						obj.options.charAppearSpeed = 0;
						obj.spanElement.innerHTML = '';
					}
				}, j * obj.options.charAppearSpeed);
				obj.timeouts.push(appearingTimeout);
			}
			var currentCharIndex1 = currentWordChars.length;
			var currentWordChars = currentWord.split('');
			for(var z = currentWordChars.length; z > 0; z--){
				if(obj.words.length - 1 > wordIndex){
					var disappearingTimeout = setTimeout(function(){
						currentCharIndex1--;
						currentWordChars.splice(currentCharIndex1, 1);
						obj.spanElement.innerHTML = '';
						obj.spanElement.innerHTML = currentWordChars.join("");
						if(obj.options.stop){
							obj.options.charAppearSpeed = 0;
							obj.options.charDissapearSpeed = 0;
							obj.options.afterWordCompleteTimeout = 0;
							obj.spanElement.innerHTML = '';
						}
					}, ((currentWordChars.length - z) * obj.options.charDissapearSpeed) + ((currentWordChars.length * obj.options.charAppearSpeed) + obj.options.afterWordCompleteTimeout));
					obj.timeouts.push(disappearingTimeout);
				}
			}
			wordIndex++;
		}, obj.wordTimeout);
		obj.timeouts.push(wordTimeout);
	}
}