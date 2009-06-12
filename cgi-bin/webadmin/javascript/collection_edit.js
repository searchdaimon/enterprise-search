
function UserPrefixSugg(authSelect, authInput, prefixInput) {
	this.prefixInput = prefixInput;
	this.authSelect = authSelect;
	this.authInput = authInput;

	this.listenToAuth();
};

UserPrefixSugg.prototype.listenToAuth = function() {
	
	var thisObj = this;

	$(this.authInput).keyup(function() {
		var username = $(thisObj.authInput).attr('value');
		thisObj.updPrefix(username);
	});
	
	$(this.authSelect).change(function () {
		username = $(thisObj.authSelect).children(":selected").text();
		thisObj.updPrefix(username);
	});
}

UserPrefixSugg.prototype.updPrefix = function(username) {
	if (username == null)
		return;
	var domainRegex = /^(.*\\)./;
	var match = username.match(domainRegex);
	if (!match) 
		return;
	var domain = match[1];

	if (this.prefixInput.attr('value') == domain)
		return;

	this.prefixInput.attr('value', domain);
	this.prefixInput.effect("highlight", { }, 3000);
}
