function mj(url, starttimeout, mintimeout, maxtimeout) {
    this.url= url;
    this.mintimeout= mintimeout? mintimeout: 250;
    this.maxtimeout= maxtimeout? maxtimeout: 64000;
    if( this.mintimeout > this.maxtimeout ) this.maxtimeout= this.mintimeout*256
    this.timeout= starttimeout? starttimeout : Math.sqrt(this.maxtimeout*this.mintimeout);
    this.onChangeTimeout();
    this.ajax= new XMLHttpRequest();
    this.ajax.httpfail= 0;
    this.queue= [];
}
mj.prototype.onChangeTimeout= function() {}
mj.prototype.rq= function(responseType, dataFilter, onsuccess, onerror, method, data) {
    if(method === undefined) { method="GET"; }
    var that= this, request;
    let onHttpError= function(e) {
        that.ajax.onreadystatechange= undefined;
        that.ajax.abort();
        if(++that.ajax.httpfail > 3) {
            if(onerror) onerror(e);
            if(that.queue.length) that.queue.shift()();
            return;
        } else {
            return request();
        }
    }
    let onreadystatechange= function() {
        if(this.readyState==4 && this.status==200) {
            let newdata;
            try {
                newdata= dataFilter(this);
                this.httpfail= 0;
            } catch(e) {
                return onHttpError(e);
            }
            this.onreadystatechange= undefined;
            onsuccess(newdata);
            if(that.timeout > that.mintimeout && (new Date - that.started)*4 < that.timeout) {
                that.timeout/=2;
                that.onChangeTimeout();
            }
            if(that.queue.length) that.queue.shift()();
            return;
        } else if( this.status==200 || this.status==0 && this.readyState!=4 ) return;
        return onHttpError(this);
    };
    request= function() {
      that.ajax.abort();
      that.ajax.onreadystatechange= onreadystatechange;
      that.started= new Date;
      that.ajax.ontimeout= function() {
          if(that.timeout<that.maxtimeout) {
              that.timeout*=2;
              that.onChangeTimeout();
          }
      }
      try {
        that.ajax.open(method, that.url, true);
        that.ajax.timeout= that.timeout;
        that.ajax.responseType = responseType;
        if(method != "GET") {
            that.ajax.setRequestHeader('Content-Type', 'application/octet-stream');
            that.ajax.send(data);
        } else that.ajax.send();
      } catch(e) {
        onHttpError();
      }
    }
    if(that.ajax.onreadystatechange) {
        that.queue.push(request);
    } else {
        request();
    }
};
mj.prototype.req= function(onsuccess, onerror, method, data) {
  return this.rq("", function(obj){return JSON.parse(obj.responseText); }, onsuccess, onerror, method, data)
}
mj.prototype.binrq= function(onsuccess, onerror, method, data) {
  return this.rq("arraybuffer", function(obj){return obj.response; }, onsuccess, onerror, method, data)
}
mj.applyJSON= function (newdata, el, prefix, level) {
    if(el==undefined) el= document;
    if(typeof newdata == "object")
        if(newdata instanceof Array) mj.applyArray(newdata, el, prefix, level);
        else mj.applyObject(newdata, el, prefix, level);
    else {
        if(el.hasAttribute("data-no-update")) return;
        switch(typeof el.getAttribute("data-format")) {
        case 'undefined': break;
        case 'string': newdata= el.getAttribute("data-format").sprintf(newdata); break;
        }
        switch(el.type) {
        case undefined: el.innerHTML= newdata; break;
        case 'checkbox': el.checked= newdata; break;
        default: el.value= newdata; break;
        }
    }
};
mj.prepareTag= function(src, els, el, attrname, prefix) { // разбор элементов для изменений
    let attr=el.getAttribute(attrname), r;
    if(attr && attr.startsWith(prefix) && (r=attr.substr(prefix.length).match(/^(\d+)(-(\d*))?$/))) {
        if(r) {
            let i=parseInt(r[1], 10);
            if(r[2]) { // '-' - признак повтора может быть в конце или с конечным индексом после него
                el._cloned = true;
                let cloned = el.cloneNode(true);
                cloned.saved_parentNode = el.parentNode;
                cloned.saved_nextSibling = el.nextSibling;
                cloned._prefix = attr;
                if(r[3]) cloned._max = parseInt(r[3], 10); // запоминаем границы повтора
                if (src[i] === undefined) src[i]=[];
                src[i].push(cloned); // и копируем тег, чтобы потом размножать по необходимости
            }
            if (els[i] == undefined) els[i]=[]; // в любом случае запоминаем место для записи значений
            el._prefix = attr;
            els[i].push(el);
        }
        return true;
    }
    return false;
}
mj.applyArray= function (newdata, p, prefix, level) {
    if(newdata.length == 2 && p.getAttribute("multiple") !== null
      && p.getAttribute("type") == "range" && p.valueLow != undefined) {
        if(!p.hasAttribute("data-no-update")) {
            p.valueLow = newdata[0];
            p.valueHigh = newdata[1];
        }
        return;
    }
    if(prefix==undefined) prefix="_";
    if(level==undefined) level=0;
    let els, src;
    if(p._els==undefined) {
      p._els = [];
      p._src = [];
    }
    if(p._els[level]==undefined) { // при первом применении определяем список изменяемых тегов и тэгов для клонирования
        els=[];
        src=[];
        for(let el of p.querySelectorAll(
         '[data-name^="'+prefix+'0"],[id^="'+prefix+'0"],[name^="'+prefix+'0"],[data-name^="'+
                         prefix+'1"],[id^="'+prefix+'1"],[name^="'+prefix+'1"],[data-name^="'+
                         prefix+'2"],[id^="'+prefix+'2"],[name^="'+prefix+'2"],[data-name^="'+
                         prefix+'3"],[id^="'+prefix+'3"],[name^="'+prefix+'3"],[data-name^="'+
                         prefix+'4"],[id^="'+prefix+'4"],[name^="'+prefix+'4"],[data-name^="'+
                         prefix+'5"],[id^="'+prefix+'5"],[name^="'+prefix+'5"],[data-name^="'+
                         prefix+'6"],[id^="'+prefix+'6"],[name^="'+prefix+'6"],[data-name^="'+
                         prefix+'7"],[id^="'+prefix+'7"],[name^="'+prefix+'7"],[data-name^="'+
                         prefix+'8"],[id^="'+prefix+'8"],[name^="'+prefix+'8"],[data-name^="'+
                         prefix+'9"],[id^="'+prefix+'9"],[name^="'+prefix+'9"]')) {
            mj.prepareTag(src, els, el, "data-name", prefix) ||
            mj.prepareTag(src, els, el, "id", prefix) ||
            mj.prepareTag(src, els, el, "data", prefix);
        }
        p._els[level] = els;
        p._src[level] = src;
    } else { // при повторном вызове берём ранее сохранённые списки тегов
        els= p._els[level];
        src= p._src[level];
    }
    p._els[level] = [];
    // размещаем данные по элементам
    for(let i=0; i<newdata.length; i++) {
        let applied=false;
        if(els[i] != undefined) { // для фиксированных или ранее присутствующих элементов
            for(let el of els[i]) {
                mj.applyJSON(newdata[i], el, el._prefix+"_", level+1);
                applied=true;
            }
            p._els[level][i] = els[i];
            els[i] = undefined;
        }
        if(applied) continue;
        let cl= p._els[level][i]? p._els[level][i]: [];
        for(let [lo, sr] of src.entries()) { // для новых размножающихся элементов
            if(i<lo) break;
            if(sr != undefined) for(let s of sr) {
                if(s._max == undefined || i<=s._max) { // данный элемент нужно размножить для заполнения
                    let el = s.cloneNode(true);
                    el._prefix = s._prefix;
                    el._cloned = true;
                    mj.replaceAttrs(el, "data-name", s._prefix, prefix+i);
                    mj.replaceAttrs(el, "id", s._prefix, prefix+i);
                    mj.replaceAttrs(el, "name", s._prefix, prefix+i);
                    mj.applyJSON(newdata[i], el, prefix+i+'_', level+1);
                    if(s.saved_nextSibling) s.saved_parentNode.insertBefore(el, s.saved_nextSibling);
                    else s.saved_parentNode.appendChild(el);
                    cl.push(el);
                }
            }
        }
        if(cl) p._els[level][i]= cl;
    }
    // удаление старых рзмноженных элементов, для которых нет соотвествия в новых данных
    for(let el of els) {
        if(el == undefined) continue;
        for(let e of el) {
            if(e._cloned) e.parentNode.removeChild(e);
        }
    }
};
mj.replaceAttrs= function (p, attrname, prefixq, prefixs) {
    let els= p.querySelectorAll('['+attrname+'^="'+prefixq+'"]');
    for(let i=0; i<els.length; i++)
        els[i].setAttribute(attrname, prefixs+els[i].getAttribute(attrname).substr(prefixq.length));
    let attr=p.getAttribute(attrname);
    if(attr!=null && attr.startsWith(prefixq)) p.setAttribute(attrname, prefixs + attr.substr(prefixq.length));
};
mj.applyObject= function (newdata, p, prefix, level) {
    if(prefix==undefined) prefix="";
    let updatedir= p.updatedir===undefined? {}:p.updatedir;
    for(let i in newdata) {
        let els= updatedir[i];
        if(els===undefined) {
            updatedir[i]= els= p.querySelectorAll('[data-name="'+prefix+i+'"],#'+prefix+i+',[name="'+prefix+i+'"]');
        }
        for(let d=0; d<els.length; d++) {
            let el= els[d];
            mj.applyJSON(newdata[i], el, prefix+i+"_", level);
        }
    }
    p.updatedir= updatedir;
};
if(String.prototype.vsprintf===undefined) {
  String.prototype.vsprintf= function(arr) {
    let i= 0;
    function callback(ex, flags, w, prec, subtype, type) {
        if (ex=='%%') return '%';
        if (arr[i]===undefined) return undefined;
        let v= arr[i++], sp= flags.indexOf('0')+1? '0':'\u00a0';
        w= w=='*'? arr[i++] : w!==undefined? parseInt(w) : 0;
        prec= prec=='*'? arr[i++] : prec!==undefined? parseInt(prec.substr(1)) : undefined;
        let val;
        switch (type[type.length-1]) {
            case 's': val= prec===undefined? v: v.substr(0, prec); break;
            case 'c': val= v[0]; break;
            case 'f': val= typeof(v)=='number'? v.toFixed(prec) : v; break;
            case 'k': v= parseFloat(v);
                {
                    if(prec===undefined) prec=6;
                    else if(!prec) prec=3;
                    let p= prec<4? 1: Math.pow(0.1, prec-3);
                    let a= Math.abs(v);
                    if((a+.5)*p>=1e3) if((a+5e5)*p>=1e9) if((a+5e8)*p>=1e12) {v*=1e-12; p='T';}
                                                         else {v*=1e-9; p='G';}
                                      else if((a+5e2)*p>=1e6) {v*=1e-6; p='M';}
                                           else {v*=1e-3; p='k';}
                        else if(a>=1e-3 - 5e-6*p) if(a>=1 - 5e-3*p) p='';
                                                  else {v*=1e3; p='m';}
                             else if(a>=1e-6 - 5e-9*p) {v*=1e6; p='µ';}
                                  else if(a>=1e-9 - 5e-12*p) {v*=1e9; p='n';}
                                       else {v=0; p='';}
                        val= v.toPrecision(prec) + type.substr(0,type.length-1).replace(' ','\u00a0') + p;
                }
                break;
            case 'g': if(prec===undefined) prec=6; else if(!prec) prec=1;
                    val= typeof(v)=='number'? v.toPrecision(prec) : v; break;
            case 'e': val= typeof(v)=='number'? v.toExponential(prec) : v; break;
            case 'x': val= parseInt(v).toString(16); break;
            case 'o': val= parseInt(v).toString(8); break;
            case 'i': case 'd': val= parseInt(v).toString(); break;
        }
        val= typeof(val)=='object' ? JSON.stringify(val) : val.toString(10);
        if(val.length<w) val= flags.indexOf('-')+1? val+'\u00a0'.repeat(w-val.length) : sp=='0'&&val[0]=='-'?
            '-'+sp.repeat(w-val.length)+val.substr(1) : sp.repeat(w-val.length)+val;
       return val;
    }
    return this.replace(String.prototype.vsprintf.regex, callback);
  };
  String.prototype.vsprintf.regex=
    /%(?:([-+ '0I#]*)([1-9][0-9]*|[*])?([.](?:[0-9]*|[*]))?(hh|ll|[hlLjzt])?([ \u00a0]?[diouxXeEfFgGaAcCSpskK])|%)/g; //'
}
if(String.prototype.sprintf===undefined)
String.prototype.sprintf= function() {
    return this.vsprintf(Array.prototype.slice.call(arguments));
};
if(String.prototype.sscanf===undefined)
String.prototype.sscanf= function(arg) {
    let f=this.split(String.prototype.vsprintf.regex);
    if(f.length==1) return arg==this;
    let r=[];
    for(let i=0; i<f.length-5; i+=6) {
        let v;
        if(arg.startsWith(f[i])) arg=arg.substr(f[i].length);
        if(!f[i+5]) {
            if(arg.startsWith('%')) arg=arg.substr(1);
            continue;
        }
        switch(f[i+5][f[i+5].length-1]) {
        case 'k':
        case 'f': case 'e': case 'g':
            [,v,arg]= arg.match(/(^\s*[+-]?\d*\.?\d*(?:e[+-]?\d*)?)(.*)/);
            v= Number(v);
            if(f[i+5][f[i+5].length-1]=='k') {
                let k;
                [,k,arg]= arg.match(/^\s*([TGMkmµn]?)(.*)/);
                switch(k) {
                case 'T': v*=1000;
                case 'G': v*=1000;
                case 'M': v*=1000;
                case 'k': v*=1000; break;
                case 'n': v/=1000;
                case 'µ': v/=1000;
                case 'm': v/=1000; break;
                }
            }
            break;
        case 'x':
            [,v,arg]= arg.match(/^\s*([+-]?(?:0x)?[0-9a-fA-F]*)(.*)/);
            v= parseInt(v, 16);
            break;
        case 'o':
            [,v,arg]= arg.match(/^\s*([+-]?[0-7]*)(.*)/);
            v= parseInt(v, 8);
            break;
        case 'd':
            [,v,arg]= arg.match(/^\s*([+-]?[0-9]*)(.*)/);
            v= parseInt(v, 10);
            break;
        case 'i':
            let x,o,d;
            [,v,x,o,d,arg]= arg.match(/^\s*([+-]?(?:(0x[0-9a-fA-F]+)|(0[0-7]+)|([0-9]*)))(.*)/);
            if(x) v=parseInt(v, 16);
            else if(o) v=parseInt(v, 8);
            else v=parseInt(v, 10);
            break;
        }
        r.push(v);
    }
    return r;
};
if (!String.prototype.repeat)
String.prototype.repeat= function(count) {
    return new Array(count+1).join(this);
};
if (!Array.prototype.find) {
  Array.prototype.find = function(predicate) {
    if (this == null) {
      throw new TypeError('Array.prototype.find called on null or undefined');
    }
    if (typeof predicate !== 'function') {
      throw new TypeError('predicate must be a function');
    }
    let list = Object(this);
    let length = list.length >>> 0;
    let thisArg = arguments[1];
    let value;

    for (let i = 0; i < length; i++) {
      value = list[i];
      if (predicate.call(thisArg, value, i, list)) {
        return value;
      }
    }
    return undefined;
  };
}
