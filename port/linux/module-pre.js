
function setArguments(href) {
    var arrTemp = href.split('?');
    if (arrTemp.length === 2) {
        var args = arrTemp[1].split('&');
        this.arguments = [];
        for (var i = 0; i < args.length; i++) {
            var argc = args[i];
            var equals = argc.indexOf('=');
            if (equals > -1) {
                this.arguments.push(argc.substring(0, equals));
                this.arguments.push(argc.substring(equals + 1));
            } else {
                this.arguments.push(argc);
            }
        }
    }
}

function handleOrientation() {
    var div = document.getElementById('emscripten_border');
    var height = window.innerHeight - 16;
    var width = window.innerWidth - 8;
    if (height > width) {
        // portrait mode, fit to width
        div.setAttribute('style', 'width:' + width + 'px');
    } else {
        // landscape mode, fit to height
        div.setAttribute('style', 'height:' + height + 'px');
    }
}

// allow passing argulements via chaos.html?[args]
setArguments(self.location.href);
if (screen.addEventListener !== undefined) {
    screen.addEventListener('mozorientationchange', handleOrientation);
}
