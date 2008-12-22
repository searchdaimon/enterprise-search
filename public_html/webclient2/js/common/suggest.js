function addAutocomplete(inputObj, submitBtn) {
    inputObj.autocomplete("suggest", 
        {   
            cacheLenght:1,
            matchSubset:0,
            onItemSelect: function(li) { $(submitBtn).click(); },
            onKbSelect: function (li) { $(inputObj).val(li.selectValue); }
        });
}

