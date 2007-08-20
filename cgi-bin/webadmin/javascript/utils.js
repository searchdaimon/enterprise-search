
function set_checked(prefix, checked) {

    var i = 0;
    var checkBox;
    while (checkBox = document.getElementById(prefix + "" + i)) {
        i++;
        checkBox.checked = checked;
    }
}
