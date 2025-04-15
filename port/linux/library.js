
mergeInto(LibraryManager.library, {
    js_save: function(key_, data_) {
        try {
            var key = UTF8ToString(key_);
            var data = UTF8ToString(data_);
            window.localStorage.setItem(key, data);
        } catch (e) {
            console.log('error in js_save:' + e);
        }
    },
    js_load: function(key_) {
        try {
            var key = UTF8ToString(key_);
            var result_ = window.localStorage.getItem(key);
            if (result_ === null)
                return 0;
            var tmp = intArrayFromString(result_);
            var result = allocate(tmp, 'i8', ALLOC_NORMAL);
            return result;
        } catch (e) {
            console.log('error in js_load:' + e);
            return 0;
        }
    },
    js_has_saved_game: function(key_) {
        try {
            var key = UTF8ToString(key_);
            var result_ = window.localStorage.getItem(key);
            if (result_ === undefined || result_ === null)
                return 0;
            return 1;
        } catch (e) {
            console.log('error in js_has_saved_game:' + e);
            return 0;
        }
    },
    js_get_window_height: function() {
        return window.innerHeight - 16;
    },
    js_get_window_width: function() {
        return window.innerWidth - 8;
    },
    js_scale_height: function() {
        handleOrientation();
    },
    js_get_lang: function() {
        var lang = navigator.language || navigator.userLanguage;
        var result = allocate(intArrayFromString(lang), 'i8', ALLOC_NORMAL);
        return result;
    }
});
