/*!
 * iro.js v4.4.0
 * 2016-2019 James Daniel
 * Licensed under MPL 2.0
 * github.com/jaames/iro.js
 */
!function (t, e) { "object" == typeof exports && "undefined" != typeof module ? module.exports = e() : "function" == typeof define && define.amd ? define(e) : t.iro = e() }(this, function () { "use strict"; var c = function () { }, i = {}, h = [], u = []; function p(t, e) { var o, n, r, i, s = arguments, a = u; for (i = arguments.length; 2 < i--;)h.push(s[i]); for (e && null != e.children && (h.length || h.push(e.children), delete e.children); h.length;)if ((n = h.pop()) && void 0 !== n.pop) for (i = n.length; i--;)h.push(n[i]); else "boolean" == typeof n && (n = null), (r = "function" != typeof t) && (null == n ? n = "" : "number" == typeof n ? n = String(n) : "string" != typeof n && (r = !1)), r && o ? a[a.length - 1] += n : a === u ? a = [n] : a.push(n), o = r; var l = new c; return l.nodeName = t, l.children = a, l.attributes = null == e ? void 0 : e, l.key = null == e ? void 0 : e.key, l } function M(t, e) { for (var o in e) t[o] = e[o]; return t } function l(t, e) { null != t && ("function" == typeof t ? t(e) : t.current = e) } var e = "function" == typeof Promise ? Promise.resolve().then.bind(Promise.resolve()) : setTimeout, f = /acit|ex(?:s|g|n|p|$)|rph|ows|mnc|ntw|ine[ch]|zoo|^ord/i, o = []; function s(t) { !t._dirty && (t._dirty = !0) && 1 == o.push(t) && e(n) } function n() { for (var t; t = o.pop();)t._dirty && I(t) } function S(t, e) { return t.normalizedNodeName === e || t.nodeName.toLowerCase() === e.toLowerCase() } function E(t) { var e = M({}, t.attributes); e.children = t.children; var o = t.nodeName.defaultProps; if (void 0 !== o) for (var n in o) void 0 === e[n] && (e[n] = o[n]); return e } function N(t) { var e = t.parentNode; e && e.removeChild(t) } function v(t, e, o, n, r) { if ("className" === e && (e = "class"), "key" === e); else if ("ref" === e) l(o, null), l(n, t); else if ("class" !== e || r) if ("style" === e) { if (n && "string" != typeof n && "string" != typeof o || (t.style.cssText = n || ""), n && "object" == typeof n) { if ("string" != typeof o) for (var i in o) i in n || (t.style[i] = ""); for (var i in n) t.style[i] = "number" == typeof n[i] && !1 === f.test(i) ? n[i] + "px" : n[i] } } else if ("dangerouslySetInnerHTML" === e) n && (t.innerHTML = n.__html || ""); else if ("o" == e[0] && "n" == e[1]) { var s = e !== (e = e.replace(/Capture$/, "")); e = e.toLowerCase().substring(2), n ? o || t.addEventListener(e, d, s) : t.removeEventListener(e, d, s), (t._listeners || (t._listeners = {}))[e] = n } else if ("list" !== e && "type" !== e && !r && e in t) { try { t[e] = null == n ? "" : n } catch (t) { } null != n && !1 !== n || "spellcheck" == e || t.removeAttribute(e) } else { var a = r && e !== (e = e.replace(/^xlink:?/, "")); null == n || !1 === n ? a ? t.removeAttributeNS("http://www.w3.org/1999/xlink", e.toLowerCase()) : t.removeAttribute(e) : "function" != typeof n && (a ? t.setAttributeNS("http://www.w3.org/1999/xlink", e.toLowerCase(), n) : t.setAttribute(e, n)) } else t.className = n || "" } function d(t) { return this._listeners[t.type](t) } var H = [], T = 0, g = !1, _ = !1; function P() { for (var t; t = H.shift();)t.componentDidMount && t.componentDidMount() } function A(t, e, o, n, r, i) { T++ || (g = null != r && void 0 !== r.ownerSVGElement, _ = null != t && !("__preactattr_" in t)); var s = U(t, e, o, n, i); return r && s.parentNode !== r && r.appendChild(s), --T || (_ = !1, i || P()), s } function U(t, e, o, n, r) { var i = t, s = g; if (null != e && "boolean" != typeof e || (e = ""), "string" == typeof e || "number" == typeof e) return t && void 0 !== t.splitText && t.parentNode && (!t._component || r) ? t.nodeValue != e && (t.nodeValue = e) : (i = document.createTextNode(e), t && (t.parentNode && t.parentNode.replaceChild(i, t), O(t, !0))), i.__preactattr_ = !0, i; var a, l, c = e.nodeName; if ("function" == typeof c) return function (t, e, o, n) { var r = t && t._component, i = r, s = t, a = r && t._componentConstructor === e.nodeName, l = a, c = E(e); for (; r && !l && (r = r._parentComponent);)l = r.constructor === e.nodeName; r && l && (!n || r._component) ? (j(r, c, 3, o, n), t = r.base) : (i && !a && (L(i), t = s = null), r = R(e.nodeName, c, o), t && !r.nextBase && (r.nextBase = t, s = null), j(r, c, 1, o, n), t = r.base, s && t !== s && (s._component = null, O(s, !1))); return t }(t, e, o, n); if (g = "svg" === c || "foreignObject" !== c && g, c = String(c), (!t || !S(t, c)) && (a = c, (l = g ? document.createElementNS("http://www.w3.org/2000/svg", a) : document.createElement(a)).normalizedNodeName = a, i = l, t)) { for (; t.firstChild;)i.appendChild(t.firstChild); t.parentNode && t.parentNode.replaceChild(i, t), O(t, !0) } var h = i.firstChild, u = i.__preactattr_, p = e.children; if (null == u) { u = i.__preactattr_ = {}; for (var f = i.attributes, d = f.length; d--;)u[f[d].name] = f[d].value } return !_ && p && 1 === p.length && "string" == typeof p[0] && null != h && void 0 !== h.splitText && null == h.nextSibling ? h.nodeValue != p[0] && (h.nodeValue = p[0]) : (p && p.length || null != h) && function (t, e, o, n, r) { var i, s, a, l, c, h = t.childNodes, u = [], p = {}, f = 0, d = 0, v = h.length, g = 0, _ = e ? e.length : 0; if (0 !== v) for (var m = 0; m < v; m++) { var y = h[m], b = y.__preactattr_, w = _ && b ? y._component ? y._component.__key : b.key : null; null != w ? (f++, p[w] = y) : (b || (void 0 !== y.splitText ? !r || y.nodeValue.trim() : r)) && (u[g++] = y) } if (0 !== _) for (var m = 0; m < _; m++) { l = e[m], c = null; var w = l.key; if (null != w) f && void 0 !== p[w] && (c = p[w], p[w] = void 0, f--); else if (d < g) for (i = d; i < g; i++)if (void 0 !== u[i] && (x = s = u[i], C = r, "string" == typeof (k = l) || "number" == typeof k ? void 0 !== x.splitText : "string" == typeof k.nodeName ? !x._componentConstructor && S(x, k.nodeName) : C || x._componentConstructor === k.nodeName)) { c = s, u[i] = void 0, i === g - 1 && g--, i === d && d++; break } c = U(c, l, o, n), a = h[m], c && c !== t && c !== a && (null == a ? t.appendChild(c) : c === a.nextSibling ? N(a) : t.insertBefore(c, a)) } var x, k, C; if (f) for (var m in p) void 0 !== p[m] && O(p[m], !1); for (; d <= g;)void 0 !== (c = u[g--]) && O(c, !1) }(i, p, o, n, _ || null != u.dangerouslySetInnerHTML), function (t, e, o) { var n; for (n in o) e && null != e[n] || null == o[n] || v(t, n, o[n], o[n] = void 0, g); for (n in e) "children" === n || "innerHTML" === n || n in o && e[n] === ("value" === n || "checked" === n ? t[n] : o[n]) || v(t, n, o[n], o[n] = e[n], g) }(i, e.attributes, u), g = s, i } function O(t, e) { var o = t._component; o ? L(o) : (null != t.__preactattr_ && l(t.__preactattr_.ref, null), !1 !== e && null != t.__preactattr_ || N(t), r(t)) } function r(t) { for (t = t.lastChild; t;) { var e = t.previousSibling; O(t, !0), t = e } } var a = []; function R(t, e, o) { var n, r = a.length; for (t.prototype && t.prototype.render ? (n = new t(e, o), y.call(n, e, o)) : ((n = new y(e, o)).constructor = t, n.render = m); r--;)if (a[r].constructor === t) return n.nextBase = a[r].nextBase, a.splice(r, 1), n; return n } function m(t, e, o) { return this.constructor(t, o) } function j(t, e, o, n, r) { t._disable || (t._disable = !0, t.__ref = e.ref, t.__key = e.key, delete e.ref, delete e.key, void 0 === t.constructor.getDerivedStateFromProps && (!t.base || r ? t.componentWillMount && t.componentWillMount() : t.componentWillReceiveProps && t.componentWillReceiveProps(e, n)), n && n !== t.context && (t.prevContext || (t.prevContext = t.context), t.context = n), t.prevProps || (t.prevProps = t.props), t.props = e, t._disable = !1, 0 !== o && (1 !== o && !1 === i.syncComponentUpdates && t.base ? s(t) : I(t, 1, r)), l(t.__ref, t)) } function I(t, e, o, n) { if (!t._disable) { var r, i, s, a = t.props, l = t.state, c = t.context, h = t.prevProps || a, u = t.prevState || l, p = t.prevContext || c, f = t.base, d = t.nextBase, v = f || d, g = t._component, _ = !1, m = p; if (t.constructor.getDerivedStateFromProps && (l = M(M({}, l), t.constructor.getDerivedStateFromProps(a, l)), t.state = l), f && (t.props = h, t.state = u, t.context = p, 2 !== e && t.shouldComponentUpdate && !1 === t.shouldComponentUpdate(a, l, c) ? _ = !0 : t.componentWillUpdate && t.componentWillUpdate(a, l, c), t.props = a, t.state = l, t.context = c), t.prevProps = t.prevState = t.prevContext = t.nextBase = null, t._dirty = !1, !_) { r = t.render(a, l, c), t.getChildContext && (c = M(M({}, c), t.getChildContext())), f && t.getSnapshotBeforeUpdate && (m = t.getSnapshotBeforeUpdate(h, u)); var y, b, w = r && r.nodeName; if ("function" == typeof w) { var x = E(r); (i = g) && i.constructor === w && x.key == i.__key ? j(i, x, 1, c, !1) : (y = i, t._component = i = R(w, x, c), i.nextBase = i.nextBase || d, i._parentComponent = t, j(i, x, 0, c, !1), I(i, 1, o, !0)), b = i.base } else s = v, (y = g) && (s = t._component = null), (v || 1 === e) && (s && (s._component = null), b = A(s, r, c, o || !f, v && v.parentNode, !0)); if (v && b !== v && i !== g) { var k = v.parentNode; k && b !== k && (k.replaceChild(b, v), y || (v._component = null, O(v, !1))) } if (y && L(y), (t.base = b) && !n) { for (var C = t, S = t; S = S._parentComponent;)(C = S).base = b; b._component = C, b._componentConstructor = C.constructor } } for (!f || o ? H.push(t) : _ || t.componentDidUpdate && t.componentDidUpdate(h, u, m); t._renderCallbacks.length;)t._renderCallbacks.pop().call(t); T || n || P() } } function L(t) { var e = t.base; t._disable = !0, t.componentWillUnmount && t.componentWillUnmount(), t.base = null; var o = t._component; o ? L(o) : e && (null != e.__preactattr_ && l(e.__preactattr_.ref, null), N(t.nextBase = e), a.push(t), r(e)), l(t.__ref, null) } function y(t, e) { this._dirty = !0, this.context = e, this.props = t, this.state = this.state || {}, this._renderCallbacks = [] } function b(t, e, o, n) { void 0 === n && (n = {}); for (var r = 0; r < e.length; r++)t.addEventListener(e[r], o, n) } function w(t, e, o, n) { void 0 === n && (n = {}); for (var r = 0; r < e.length; r++)t.removeEventListener(e[r], o, n) } M(y.prototype, { setState: function (t, e) { this.prevState || (this.prevState = this.state), this.state = M(M({}, this.state), "function" == typeof t ? t(this.state, this.props) : t), e && this._renderCallbacks.push(e), s(this) }, forceUpdate: function (t) { t && this._renderCallbacks.push(t), I(this, 2) }, render: function () { } }); var x = "mousedown", k = "mousemove", C = "mouseup", D = "touchstart", W = "touchmove", B = "touchend", t = function (e) { function t(t) { e.call(this, t), this.uid = (Math.random() + 1).toString(36).substring(5) } return e && (t.__proto__ = e), ((t.prototype = Object.create(e && e.prototype)).constructor = t).prototype.componentDidMount = function () { b(this.base, [x, D], this, { passive: !1 }) }, t.prototype.componentWillUnmount = function () { w(this.base, [x, D], this) }, t.prototype.handleEvent = function (t) { t.preventDefault(); var e = t.touches ? t.changedTouches[0] : t, o = e.clientX, n = e.clientY, r = this.base.getBoundingClientRect(); switch (t.type) { case x: case D: b(document, [k, W, C, B], this, { passive: !1 }), this.handleInput(o, n, r, "START"); break; case k: case W: this.handleInput(o, n, r, "MOVE"); break; case C: case B: this.handleInput(o, n, r, "END"), w(document, [k, W, C, B], this, { passive: !1 }) } }, t }(y); function V(t) { var e = window.navigator.userAgent, o = /^((?!chrome|android).)*safari/i.test(e), n = /iPhone|iPod|iPad/i.test(e), r = window.location; return o || n ? r.protocol + "//" + r.host + r.pathname + r.search + t : t } function F(t, e, o, n, r) { var i = r - n <= 180 ? 0 : 1; return n *= Math.PI / 180, r *= Math.PI / 180, "M " + (t + o * Math.cos(r)) + " " + (e + o * Math.sin(r)) + " A " + o + " " + o + " 0 " + i + " 0 " + (t + o * Math.cos(n)) + " " + (e + o * Math.sin(n)) } function $(t) { var e = t.r, o = t.url; return p("svg", { class: "iro__handle", x: t.x, y: t.y, style: { overflow: "visible" } }, o && p("use", Object.assign({}, { xlinkHref: V(o) }, t.origin)), !o && p("circle", { class: "iro__handle__inner", r: e, fill: "none", "stroke-width": 2, stroke: "#000" }), !o && p("circle", { class: "iro__handle__outer", r: e - 2, fill: "none", "stroke-width": 2, stroke: "#fff" })) } $.defaultProps = { x: 0, y: 0, r: 8, url: null, origin: { x: 0, y: 0 } }; var G = Array.apply(null, { length: 360 }).map(function (t, e) { return e }), z = function (t) { function e() { t.apply(this, arguments) } return t && (e.__proto__ = t), ((e.prototype = Object.create(t && t.prototype)).constructor = e).prototype._transformAngle = function (t, e) { var o = this.props.wheelAngle; return ((t = "clockwise" === this.props.wheelDirection ? -360 + t - (e ? -o : o) : o - t) % 360 + 360) % 360 }, e.prototype.render = function (t) { var e = this, o = t.width, n = t.borderWidth, r = t.handleRadius, i = t.color.hsv, s = o / 2 - n, a = this._transformAngle(i.h, !0) * (Math.PI / 180), l = i.s / 100 * (s - t.padding - r - n), c = s + n, h = s + n; return p("svg", { class: "iro__wheel", width: o, height: o, style: { overflow: "visible", display: "block" } }, p("defs", null, p("radialGradient", { id: this.uid }, p("stop", { offset: "0%", "stop-color": "#fff" }), p("stop", { offset: "100%", "stop-color": "#fff", "stop-opacity": "0" }))), p("g", { class: "iro__wheel__hue", "stroke-width": s, fill: "none" }, G.map(function (t) { return p("path", { key: t, d: F(c, h, s / 2, t, t + 1.5), stroke: "hsl(" + e._transformAngle(t) + ", 100%, 50%)" }) })), p("circle", { class: "iro__wheel__saturation", cx: c, cy: h, r: s, fill: "url(" + V("#" + this.uid) + ")" }), t.wheelLightness && p("circle", { class: "iro__wheel__lightness", cx: c, cy: h, r: s, fill: "#000", opacity: 1 - i.v / 100 }), p("circle", { class: "iro__wheel__border", cx: c, cy: h, r: s, fill: "none", stroke: t.borderColor, "stroke-width": n }), p($, { r: r, url: t.handleSvg, origin: t.handleOrigin, x: c + l * Math.cos(a), y: h + l * Math.sin(a) })) }, e.prototype.handleInput = function (t, e, o, n) { var r = o.left, i = o.top, s = this.props, a = s.width / 2, l = a - s.padding - s.handleRadius - s.borderWidth; t = a - (t - r), e = a - (e - i); var c = Math.atan2(e, t), h = this._transformAngle(Math.round(c * (180 / Math.PI)) + 180), u = Math.min(Math.sqrt(t * t + e * e), l); s.onInput(n, { h: h, s: Math.round(100 / l * u) }) }, e }(t); function q(t, e) { var o = -1 < t.indexOf("%"), n = parseFloat(t); return o ? e / 100 * n : n } function X(t) { return parseInt(t, 16) } function Y(t) { return t.toString(16).padStart(2, "0") } var J = "(?:[-\\+]?\\d*\\.\\d+%?)|(?:[-\\+]?\\d+%?)", K = "[\\s|\\(]+(" + J + ")[,|\\s]+(" + J + ")[,|\\s]+(" + J + ")\\s*\\)?", Q = "[\\s|\\(]+(" + J + ")[,|\\s]+(" + J + ")[,|\\s]+(" + J + ")[,|\\s]+(" + J + ")\\s*\\)?", Z = new RegExp("rgb" + K), tt = new RegExp("rgba" + Q), et = new RegExp("hsl" + K), ot = new RegExp("hsla" + Q), nt = "^(?:#?|0x?)", rt = "([0-9a-fA-F]{1})", it = "([0-9a-fA-F]{2})", st = new RegExp("" + nt + rt + rt + rt + "$"), at = new RegExp("" + nt + rt + rt + rt + rt + "$"), lt = new RegExp("" + nt + it + it + it + "$"), ct = new RegExp("" + nt + it + it + it + it + "$"), ht = function (t) { this._onChange = !1, this._value = { h: 0, s: 0, v: 0, a: 1 }, t && this.set(t) }, ut = { hsv: { configurable: !0 }, rgb: { configurable: !0 }, hsl: { configurable: !0 }, rgbString: { configurable: !0 }, hexString: { configurable: !0 }, hslString: { configurable: !0 } }; ht.prototype.set = function (t) { var e = "string" == typeof t, o = "object" == typeof t; if (e && /^(?:#?|0x?)[0-9a-fA-F]{3,8}$/.test(t)) this.hexString = t; else if (e && /^rgba?/.test(t)) this.rgbString = t; else if (e && /^hsla?/.test(t)) this.hslString = t; else if (o && t instanceof ht) this.hsv = t.hsv; else if (o && "r" in t && "g" in t && "b" in t) this.rgb = t; else if (o && "h" in t && "s" in t && "v" in t) this.hsv = t; else { if (!(o && "h" in t && "s" in t && "l" in t)) throw new Error("invalid color value"); this.hsl = t } }, ht.prototype.setChannel = function (t, e, o) { var n; this[t] = Object.assign({}, this[t], ((n = {})[e] = o, n)) }, ht.prototype.clone = function () { return new ht(this) }, ht.hsvToRgb = function (t) { var e = t.h / 60, o = t.s / 100, n = t.v / 100, r = Math.floor(e), i = e - r, s = n * (1 - o), a = n * (1 - i * o), l = n * (1 - (1 - i) * o), c = r % 6; return { r: 255 * [n, a, s, s, l, n][c], g: 255 * [l, n, n, a, s, s][c], b: 255 * [s, s, l, n, n, a][c] } }, ht.rgbToHsv = function (t) { var e, o = t.r / 255, n = t.g / 255, r = t.b / 255, i = Math.max(o, n, r), s = Math.min(o, n, r), a = i - s, l = i, c = 0 === i ? 0 : a / i; switch (i) { case s: e = 0; break; case o: e = (n - r) / a + (n < r ? 6 : 0); break; case n: e = (r - o) / a + 2; break; case r: e = (o - n) / a + 4 }return { h: 60 * e, s: 100 * c, v: 100 * l } }, ht.hsvToHsl = function (t) { var e = t.s / 100, o = t.v / 100, n = (2 - e) * o, r = n <= 1 ? n : 2 - n, i = r < 1e-9 ? 0 : e * o / r; return { h: t.h, s: 100 * i, l: 50 * n } }, ht.hslToHsv = function (t) { var e = 2 * t.l, o = t.s * (e <= 100 ? e : 200 - e) / 100, n = e + o < 1e-9 ? 0 : 2 * o / (e + o); return { h: t.h, s: 100 * n, v: (e + o) / 2 } }, ut.hsv.get = function () { var t = this._value; return { h: t.h, s: t.s, v: t.v } }, ut.hsv.set = function (t) { var e = this._value; if (t = Object.assign({}, e, t), this._onChange) { var o = {}; for (var n in e) o[n] = t[n] != e[n]; this._value = t, (o.h || o.s || o.v || o.a) && this._onChange(this, o) } else this._value = t }, ut.rgb.get = function () { var t = ht.hsvToRgb(this._value), e = t.r, o = t.g, n = t.b; return { r: Math.round(e), g: Math.round(o), b: Math.round(n) } }, ut.rgb.set = function (t) { this.hsv = Object.assign({}, ht.rgbToHsv(t), { a: void 0 === t.a ? 1 : t.a }) }, ut.hsl.get = function () { var t = ht.hsvToHsl(this._value), e = t.h, o = t.s, n = t.l; return { h: Math.round(e), s: Math.round(o), l: Math.round(n) } }, ut.hsl.set = function (t) { this.hsv = Object.assign({}, ht.hslToHsv(t), { a: void 0 === t.a ? 1 : t.a }) }, ut.rgbString.get = function () { var t = this.rgb; return "rgb(" + t.r + ", " + t.g + ", " + t.b + ")" }, ut.rgbString.set = function (t) { var e, o, n, r, i = 1; if ((e = Z.exec(t)) ? (o = q(e[1], 255), n = q(e[2], 255), r = q(e[3], 255)) : (e = tt.exec(t)) && (o = q(e[1], 255), n = q(e[2], 255), r = q(e[3], 255), i = q(e[4], 1)), !e) throw new Error("invalid rgb string"); this.rgb = { r: o, g: n, b: r, a: i } }, ut.hexString.get = function () { var t = this.rgb; return "#" + Y(t.r) + Y(t.g) + Y(t.b) }, ut.hexString.set = function (t) { var e, o, n, r, i = 255; if ((e = st.exec(t)) ? (o = 17 * X(e[1]), n = 17 * X(e[2]), r = 17 * X(e[3])) : (e = at.exec(t)) ? (o = 17 * X(e[1]), n = 17 * X(e[2]), r = 17 * X(e[3]), i = 17 * X(e[4])) : (e = lt.exec(t)) ? (o = X(e[1]), n = X(e[2]), r = X(e[3])) : (e = ct.exec(t)) && (o = X(e[1]), n = X(e[2]), r = X(e[3]), i = X(e[4])), !e) throw new Error("invalid hex string"); this.rgb = { r: o, g: n, b: r, a: i / 255 } }, ut.hslString.get = function () { var t = this.hsl; return "hsl(" + t.h + ", " + t.s + "%, " + t.l + "%)" }, ut.hslString.set = function (t) { var e, o, n, r, i = 1; if ((e = et.exec(t)) ? (o = q(e[1], 360), n = q(e[2], 100), r = q(e[3], 100)) : (e = ot.exec(t)) && (o = q(e[1], 360), n = q(e[2], 100), r = q(e[3], 100), i = q(e[4], 1)), !e) throw new Error("invalid hsl string"); this.hsl = { h: o, s: n, l: r, a: i } }, Object.defineProperties(ht.prototype, ut); var pt = function (t) { function e() { t.apply(this, arguments) } return t && (e.__proto__ = t), ((e.prototype = Object.create(t && t.prototype)).constructor = e).prototype.renderGradient = function (t) { var e = t.color.hsv, o = []; switch (t.sliderType) { case "hue": o = [{ offset: "0", color: "#f00" }, { offset: "16.666", color: "#ff0" }, { offset: "33.333", color: "#0f0" }, { offset: "50", color: "#0ff" }, { offset: "66.666", color: "#00f" }, { offset: "83.333", color: "#f0f" }, { offset: "100", color: "#f00" }]; break; case "saturation": var n = ht.hsvToHsl({ h: e.h, s: 0, v: e.v }), r = ht.hsvToHsl({ h: e.h, s: 100, v: e.v }); o = [{ offset: "0", color: "hsl(" + n.h + ", " + n.s + "%, " + n.l + "%)" }, { offset: "100", color: "hsl(" + r.h + ", " + r.s + "%, " + r.l + "%)" }]; break; case "value": default: var i = ht.hsvToHsl({ h: e.h, s: e.s, v: 100 }); o = [{ offset: "0", color: "#000" }, { offset: "100", color: "hsl(" + i.h + ", " + i.s + "%, " + i.l + "%)" }] }return p("linearGradient", { id: this.uid }, o.map(function (t) { return p("stop", { offset: t.offset + "%", "stop-color": t.color }) })) }, e.prototype.render = function (t) { var e = t.width, o = t.sliderHeight, n = t.borderWidth, r = t.handleRadius; o = o || 2 * t.padding + 2 * r + 2 * n, this.width = e; var i, s = (this.height = o) / 2, a = e - 2 * s, l = t.color.hsv; switch (t.sliderType) { case "hue": i = l.h /= 3.6; break; case "saturation": i = l.s; break; case "value": default: i = l.v }return p("svg", { class: "iro__slider", width: e, height: o, style: { marginTop: t.sliderMargin, overflow: "visible", display: "block" } }, p("defs", null, this.renderGradient(t)), p("rect", { class: "iro__slider__value", rx: s, ry: s, x: n / 2, y: n / 2, width: e - n, height: o - n, "stroke-width": n, stroke: t.borderColor, fill: "url(" + V("#" + this.uid) + ")" }), p($, { r: r, url: t.handleSvg, origin: t.handleOrigin, x: s + i / 100 * a, y: o / 2 })) }, e.prototype.getValueFromPoint = function (t, e, o) { var n = o.left, r = this.width - this.height; t -= n + this.height / 2; var i = Math.max(Math.min(t, r), 0); return Math.round(100 / r * i) }, e.prototype.handleInput = function (t, e, o, n) { var r, i, s = this.getValueFromPoint(t, e, o); switch (this.props.sliderType) { case "hue": i = "h", s *= 3.6; break; case "saturation": i = "s"; break; case "value": default: i = "v" }this.props.onInput(n, ((r = {})[i] = s, r)) }, e }(t); var ft = function (e) { function i(t) { e.call(this, t), this.emitHook("init:before"), this._events = {}, this._deferredEvents = {}, this._colorUpdateActive = !1, this._colorUpdateSrc = null, this.color = new ht(t.color), this.deferredEmit("color:init", this.color, { h: !1, s: !1, v: !1, a: !1 }), this.color._onChange = this.updateColor.bind(this), this.state = Object.assign({}, t, { color: this.color }), this.emitHook("init:state"), t.layout ? this.layout = t.layout : this.layout = [{ component: z, options: {} }, { component: pt, options: {} }], this.emitHook("init:after") } return e && (i.__proto__ = e), ((i.prototype = Object.create(e && e.prototype)).constructor = i).prototype.on = function (t, e) { var o = this, n = this._events; (Array.isArray(t) ? t : [t]).forEach(function (t) { o.emitHook("event:on", t, e), (n[t] || (n[t] = [])).push(e), o._deferredEvents[t] && (o._deferredEvents[t].forEach(function (t) { e.apply(null, t) }), o._deferredEvents[t] = []) }) }, i.prototype.off = function (t, o) { var n = this; (Array.isArray(t) ? t : [t]).forEach(function (t) { var e = n._events[t]; n.emitHook("event:off", t, o), e && e.splice(e.indexOf(o), 1) }) }, i.prototype.emit = function (t) { for (var e, o = [], n = arguments.length - 1; 0 < n--;)o[n] = arguments[n + 1]; (e = this).emitHook.apply(e, [t].concat(o)); for (var r = this._events[t] || [], i = 0; i < r.length; i++)r[i].apply(null, o) }, i.prototype.deferredEmit = function (t) { for (var e, o = [], n = arguments.length - 1; 0 < n--;)o[n] = arguments[n + 1]; var r = this._deferredEvents; (e = this).emit.apply(e, [t].concat(o)), (r[t] || (r[t] = [])).push(o) }, i.prototype.resize = function (t) { this.setState({ width: t }) }, i.prototype.reset = function () { this.color.set(this.props.color) }, i.addHook = function (t, e) { var o = i.pluginHooks; (o[t] || (o[t] = [])).push(e) }, i.prototype.emitHook = function (t) { for (var e = [], o = arguments.length - 1; 0 < o--;)e[o] = arguments[o + 1]; for (var n = i.pluginHooks[t] || [], r = 0; r < n.length; r++)n[r].apply(this, e) }, i.prototype.onMount = function (t) { this.el = t, this.deferredEmit("mount", this) }, i.prototype.updateColor = function (t, e) { this.emitHook("color:beforeUpdate", t, e), this.setState({ color: t }), this.emitHook("color:afterUpdate", t, e), this._colorUpdateActive || (this._colorUpdateActive = !0, "input" == this._colorUpdateSrc && this.emit("input:change", t, e), this.emit("color:change", t, e), this._colorUpdateActive = !1) }, i.prototype.handleInput = function (t, e) { "START" === t && this.emit("input:start", this.color), "MOVE" === t && this.emit("input:move", this.color), this._colorUpdateSrc = "input", this.color.hsv = e, "END" === t && this.emit("input:end", this.color), this._colorUpdateSrc = null }, i.prototype.render = function (t, n) { var r = this; return p("div", { class: "iro__colorPicker", style: { display: n.display, width: n.width } }, this.layout.map(function (t) { var e = t.component, o = t.options; return p(e, Object.assign({}, n, o, { onInput: function (t, e) { return r.handleInput(t, e) }, parent: r })) })) }, i }(y); ft.pluginHooks = {}, ft.defaultProps = { width: 300, height: 300, handleRadius: 8, handleSvg: null, handleOrigin: { x: 0, y: 0 }, color: "#fff", borderColor: "#fff", borderWidth: 0, display: "block", wheelLightness: !0, wheelAngle: 0, wheelDirection: "anticlockwise", sliderHeight: null, sliderMargin: 12, padding: 6, layout: null }; var dt, vt, gt, _t, mt = ((vt = function (e, t) { var o, n, r, i = null, s = document.createElement("div"); return o = p(dt, Object.assign({}, { ref: function (t) { return i = t } }, t)), A(n, o, {}, !1, s, !1), r = function () { var t = e instanceof Element ? e : document.querySelector(e); t.appendChild(i.base), i.onMount(t) }, "loading" !== document.readyState ? r() : b(document, ["DOMContentLoaded"], r), i }).prototype = (dt = ft).prototype, Object.assign(vt, dt), vt.__component = dt, vt); return _t = [], (gt = { Color: ht, ColorPicker: mt, ui: { h: p, Component: t, Handle: $, Slider: pt, Wheel: z }, util: { resolveUrl: V, createArcPath: F, parseUnit: q, parseHexInt: X, intToHex: Y }, version: "4.4.0" }).use = function (t, e) { void 0 === e && (e = {}), -1 < _t.indexOf(t) || (t(gt, e), _t.push(t)) }, gt.installedPlugins = _t, gt });
/*!
 * RangeTouch v2.0
 * https://github.com/sampotts/rangetouch
 */
!function (e, t) { "object" == typeof exports && "undefined" != typeof module ? module.exports = t() : "function" == typeof define && define.amd ? define("RangeTouch", t) : e.RangeTouch = t() }(this, function () { "use strict"; function e(e, t) { for (var n = 0; n < t.length; n++) { var r = t[n]; r.enumerable = r.enumerable || !1, r.configurable = !0, "value" in r && (r.writable = !0), Object.defineProperty(e, r.key, r) } } var t = { addCSS: !0, thumbWidth: 15, watch: !0 }; var n = function (e) { return null != e ? e.constructor : null }, r = function (e, t) { return !!(e && t && e instanceof t) }, u = function (e) { return null == e }, i = function (e) { return n(e) === Object }, o = function (e) { return n(e) === String }, a = function (e) { return Array.isArray(e) }, c = function (e) { return r(e, NodeList) }, l = { nullOrUndefined: u, object: i, number: function (e) { return n(e) === Number && !Number.isNaN(e) }, string: o, boolean: function (e) { return n(e) === Boolean }, function: function (e) { return n(e) === Function }, array: a, nodeList: c, element: function (e) { return r(e, Element) }, event: function (e) { return r(e, Event) }, empty: function (e) { return u(e) || (o(e) || a(e) || c(e)) && !e.length || i(e) && !Object.keys(e).length } }; function s(e, t) { if (1 > t) { var n = function (e) { var t = "".concat(e).match(/(?:\.(\d+))?(?:[eE]([+-]?\d+))?$/); return t ? Math.max(0, (t[1] ? t[1].length : 0) - (t[2] ? +t[2] : 0)) : 0 }(t); return parseFloat(e.toFixed(n)) } return Math.round(e / t) * t } return function () { function n(e, r) { (function (e, t) { if (!(e instanceof t)) throw new TypeError("Cannot call a class as a function") })(this, n), l.element(e) ? this.element = e : l.string(e) && (this.element = document.querySelector(e)), l.element(this.element) && l.empty(this.element.rangeTouch) && (this.config = Object.assign({}, t, r), this.init()) } return r = n, i = [{ key: "setup", value: function (e) { var r = 1 < arguments.length && void 0 !== arguments[1] ? arguments[1] : {}, u = null; if (l.empty(e) || l.string(e) ? u = Array.from(document.querySelectorAll(l.string(e) ? e : 'input[type="range"]')) : l.element(e) ? u = [e] : l.nodeList(e) ? u = Array.from(e) : l.array(e) && (u = e.filter(l.element)), l.empty(u)) return null; var i = Object.assign({}, t, r); l.string(e) && i.watch && new MutationObserver(function (t) { Array.from(t).forEach(function (t) { Array.from(t.addedNodes).forEach(function (t) { l.element(t) && function (e, t) { return function () { return Array.from(document.querySelectorAll(t)).includes(this) }.call(e, t) }(t, e) && new n(t, i) }) }) }).observe(document.body, { childList: !0, subtree: !0 }); return u.map(function (e) { return new n(e, r) }) } }, { key: "enabled", get: function () { return "ontouchstart" in document.documentElement } }], (u = [{ key: "init", value: function () { n.enabled && (this.config.addCSS && (this.element.style.userSelect = "none", this.element.style.webKitUserSelect = "none", this.element.style.touchAction = "manipulation"), this.listeners(!0), this.element.rangeTouch = this) } }, { key: "destroy", value: function () { n.enabled && (this.listeners(!1), this.element.rangeTouch = null) } }, { key: "listeners", value: function (e) { var t = this, n = e ? "addEventListener" : "removeEventListener";["touchstart", "touchmove", "touchend"].forEach(function (e) { t.element[n](e, function (e) { return t.set(e) }, !1) }) } }, { key: "get", value: function (e) { if (!n.enabled || !l.event(e)) return null; var t, r = e.target, u = e.changedTouches[0], i = parseFloat(r.getAttribute("min")) || 0, o = parseFloat(r.getAttribute("max")) || 100, a = parseFloat(r.getAttribute("step")) || 1, c = r.getBoundingClientRect(), f = 100 / c.width * (this.config.thumbWidth / 2) / 100; return 0 > (t = 100 / c.width * (u.clientX - c.left)) ? t = 0 : 100 < t && (t = 100), 50 > t ? t -= (100 - 2 * t) * f : 50 < t && (t += 2 * (t - 50) * f), i + s(t / 100 * (o - i), a) } }, { key: "set", value: function (e) { n.enabled && l.event(e) && !e.target.disabled && (e.preventDefault(), e.target.value = this.get(e), function (e, t) { if (e && t) { var n = new Event(t); e.dispatchEvent(n) } }(e.target, "touchend" === e.type ? "change" : "input")) } }]) && e(r.prototype, u), i && e(r, i), n; var r, u, i }() });
//page js
var loc = false, locip;
var ps = false, noNewSegs = false;
var isOn = false, nlA = false, isLv = false, isInfo = false, syncSend = false, syncTglRecv = true, isRgbw = false;
var whites = [0, 0, 0];
var expanded = [false];
var powered = [true];
var nlDur = 60, nlTar = 0;
var nlFade = false;
var selectedFx = 0;
var csel = 0;
var savedPresets = 0;
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0;
var d = document;
const ranges = RangeTouch.setup('input[type="range"]', {});
var lastinfo = {};
var cfg = {
    theme: { base: "dark", bg: { url: "" }, alpha: { bg: 0.6, tab: 0.8 }, color: { bg: "" } },
    comp: { colors: { picker: true, rgb: false, quick: true, hex: false }, labels: true, pcmbot: false }
};

var cpick = new iro.ColorPicker("#picker", {
    width: 260,
    wheelLightness: false
});

function handleVisibilityChange() {
    if (!document.hidden && new Date() - lastUpdate > 3000) {
        requestJson(null);
    }
}

function sCol(na, col) {
    d.documentElement.style.setProperty(na, col);
}

function applyCfg() {
    cTheme(cfg.theme.base === "light");
    var bg = cfg.theme.color.bg;
    if (bg) sCol('--c-1', bg);
    var ccfg = cfg.comp.colors;
    d.getElementById('hexw').style.display = ccfg.hex ? "block" : "none";
    d.getElementById('picker').style.display = ccfg.picker ? "block" : "none";
    d.getElementById('rgbwrap').style.display = ccfg.rgb ? "block" : "none";
    d.getElementById('qcs-w').style.display = ccfg.quick ? "block" : "none";
    var l = cfg.comp.labels;
    var e = d.querySelectorAll('.tab-label');
    for (var i = 0; i < e.length; i++)
        e[i].style.display = l ? "block" : "none";
    e = d.querySelector('.hd');
    e.style.display = l ? "block" : "none";
    sCol('--tbp', l ? "14px 14px 10px 14px" : "10px 22px 4px 22px");
    sCol('--bbp', l ? "9px 0 7px 0" : "10px 0 4px 0");
    sCol('--bhd', l ? "block" : "none");
    sCol('--bmt', l ? "0px" : "5px");
    sCol('--t-b', cfg.theme.alpha.tab);
    size();
    localStorage.setItem('wledUiCfg', JSON.stringify(cfg));
}

function tglHex() {
    cfg.comp.colors.hex = !cfg.comp.colors.hex;
    applyCfg();
}

function tglTheme() {
    cfg.theme.base = (cfg.theme.base === "light") ? "dark" : "light";
    applyCfg();
}

function tglLabels() {
    cfg.comp.labels = !cfg.comp.labels;
    applyCfg();
}

function cTheme(light) {
    if (light) {
        sCol('--c-1', '#eee');
        sCol('--c-f', '#000');
        sCol('--c-2', '#ddd');
        sCol('--c-3', '#bbb');
        sCol('--c-4', '#aaa');
        sCol('--c-5', '#999');
        sCol('--c-6', '#999');
        sCol('--c-8', '#888');
        sCol('--c-b', '#444');
        sCol('--c-c', '#333');
        sCol('--c-e', '#111');
        sCol('--c-d', '#222');
        sCol('--c-r', '#c42');
        sCol('--c-o', 'rgba(204, 204, 204, 0.9)');
        sCol('--c-sb', '#0003'); sCol('--c-sbh', '#0006');
        sCol('--c-tb', 'rgba(204, 204, 204, var(--t-b))');
        sCol('--c-tba', 'rgba(170, 170, 170, var(--t-b))');
        sCol('--c-tbh', 'rgba(204, 204, 204, var(--t-b))');
        d.getElementById('imgw').style.filter = "invert(0.8)";
    } else {
        sCol('--c-1', '#111');
        sCol('--c-f', '#fff');
        sCol('--c-2', '#222');
        sCol('--c-3', '#333');
        sCol('--c-4', '#444');
        sCol('--c-5', '#555');
        sCol('--c-6', '#666');
        sCol('--c-8', '#888');
        sCol('--c-b', '#bbb');
        sCol('--c-c', '#ccc');
        sCol('--c-e', '#eee');
        sCol('--c-d', '#ddd');
        sCol('--c-r', '#831');
        sCol('--c-o', 'rgba(34, 34, 34, 0.9)');
        sCol('--c-sb', '#fff3'); sCol('--c-sbh', '#fff5');
        sCol('--c-tb', 'rgba(34, 34, 34, var(--t-b))');
        sCol('--c-tba', 'rgba(102, 102, 102, var(--t-b))');
        sCol('--c-tbh', 'rgba(51, 51, 51, var(--t-b))');
        d.getElementById('imgw').style.filter = "unset";
    }
}

function loadBg(iUrl) {
    let bg = document.getElementById('bg');
    let img = document.createElement("img");
    img.src = iUrl;
    img.addEventListener('load', (event) => {
        var a = parseFloat(cfg.theme.alpha.bg);
        d.getElementById('staytop2').style.background = "transparent";
        if (isNaN(a)) a = 0.6;
        bg.style.opacity = a;
        bg.style.backgroundImage = `url(${iUrl})`;
        img = null;
    });
}

function onLoad() {
    if (window.location.protocol == "file:") {
        loc = true;
        locip = localStorage.getItem('locIp');
        if (!locip) {
            locip = prompt("File Mode. Please enter WLED IP!");
            localStorage.setItem('locIp', locip);
        }
    }
    var sett = localStorage.getItem('wledUiCfg');
    if (sett) cfg = mergeDeep(cfg, JSON.parse(sett));

    applyCfg();
    loadBg(cfg.theme.bg.url);

    var cd = d.getElementById('csl').children;
    for (i = 0; i < cd.length; i++) {
        cd[i].style.backgroundColor = "rgb(0, 0, 0)";
    }
    selectSlot(0);
    updateTablinks(0);
    resetUtil();
    cpick.on("input:end", function () {
        setColor(1);
    });
    setTimeout(function () { requestJson(null, false) }, 25);
    d.addEventListener("visibilitychange", handleVisibilityChange, false);
    size();
    d.getElementById("cv").style.opacity = 0;
    if (localStorage.getItem('pcm') == "true") togglePcMode(true);
}

function updateTablinks(tabI) {
    tablinks = d.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    if (pcMode) return;
    tablinks[tabI].className += " active";
}

function openTab(tabI, force = false) {
    if (pcMode && !force) return;
    var i, tabcontent, tablinks;
    iSlide = tabI;
    _C.classList.toggle('smooth', false);
    _C.style.setProperty('--i', iSlide);
    updateTablinks(tabI);
}

var timeout;
function showToast(text, error = false) {
    if (error) d.getElementById('connind').style.backgroundColor = "#831";
    var x = d.getElementById("toast");
    x.innerHTML = text;
    x.className = error ? "error" : "show";
    clearTimeout(timeout);
    x.style.animation = 'none';
    x.offsetHeight;
    x.style.animation = null;
    timeout = setTimeout(function () { x.className = x.className.replace("show", ""); }, 2900);
}

function showErrorToast() {
    showToast('Connection to light failed!', true);
}
function clearErrorToast() {
    d.getElementById("toast").className = d.getElementById("toast").className.replace("error", "");
}

function getRuntimeStr(rt) {
    var t = parseInt(rt);
    var days = Math.floor(t / 86400);
    var hrs = Math.floor((t - days * 86400) / 3600);
    var mins = Math.floor((t - days * 86400 - hrs * 3600) / 60);
    var str = days ? (days + " " + (days == 1 ? "day" : "days") + ", ") : "";
    str += (hrs || days) ? (hrs + " " + (hrs == 1 ? "hour" : "hours")) : "";
    if (!days && hrs) str += ", ";
    if (t > 59 && !days) str += mins + " min";
    if (t < 3600 && t > 59) str += ", ";
    if (t < 3600) str += (t - mins * 60) + " sec";
    return str;
}

function inforow(key, val, unit = "") {
    return `<tr><td class="keytd">${key}</td><td class="valtd">${val}${unit}</td></tr>`;
}

function populateInfo(i) {
    var cn = "";
    var heap = i.freeheap / 1000;
    heap = heap.toFixed(1);
    var pwr = i.leds.pwr;
    var pwru = "Not calculated";
    if (pwr > 1000) { pwr /= 1000; pwr = pwr.toFixed((pwr > 10) ? 0 : 1); pwru = pwr + " A"; }
    else if (pwr > 0) { pwr = 50 * Math.round(pwr / 50); pwru = pwr + " mA"; }
    var urows = "";
    for (var k in i.u) {
        var val = i.u[k];
        if (val[1]) {
            urows += inforow(k, val[0], val[1]);
        } else {
            urows += inforow(k, val);
        }
    }
    var vcn = "Kuuhaku";
    if (i.ver.startsWith("0.10.")) vcn = "Namigai";
    if (i.ver.startsWith("0.10.2")) vcn = "Fumikiri";
    if (i.cn) vcn = i.cn;

    cn += `v${i.ver} "${vcn}"<br><br><table class="infot">
	${urows}
	${inforow("Build", i.vid)}
	${inforow("Signal strength", i.wifi.signal + "% (" + i.wifi.rssi, " dBm)")}
	${inforow("Uptime", getRuntimeStr(i.uptime))}
	${inforow("Free heap", heap, " kB")}
	${inforow("Estimated current", pwru)}
	${inforow("MAC address", i.mac)}
	${inforow("Environment", i.arch + " " + i.core + " (" + i.lwip + ")")}
	</table>`;
    d.getElementById('kv').innerHTML = cn;
}

function populateSegments(s) {
    var cn = "";
    segCount = 0, lowestUnused = 0; lSeg = 0;

    for (y in s.seg) {
        segCount++;

        var inst = s.seg[y];
        var i = parseInt(inst.id);
        powered[i] = inst.on;
        if (i == lowestUnused) lowestUnused = i + 1;
        if (i > lSeg) lSeg = i;

        cn += `<div class="seg">
			<label class="check schkl">
				&nbsp;
				<input type="checkbox" id="seg${i}sel" onchange="selSeg(${i})" ${inst.sel ? "checked" : ""}>
				<span class="checkmark schk"></span>
			</label>
			<div class="segname" onclick="selSegEx(${i})">
				Segment ${i}
			</div>
			<i class="icons e-icon flr ${expanded[i] ? "exp" : ""}" id="sege${i}" onclick="expand(${i})">&#xe395;</i>
			<div class="segin ${expanded[i] ? "expanded" : ""}" id="seg${i}">
			<table class="segt">
				<tr>
					<td class="segtd">Start LED</td>
					<td class="segtd">Stop LED</td>
				</tr>
				<tr>
					<td class="segtd"><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${ledCount - 1}" value="${inst.start}" oninput="updateLen(${i})"></td>
					<td class="segtd"><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${ledCount}" value="${inst.stop}" oninput="updateLen(${i})"></td>
				</tr>
			</table>
			<table class="segt">
				<tr>
					<td class="segtd">Grouping</td>
					<td class="segtd">Spacing</td>
				</tr>
				<tr>
					<td class="segtd"><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})"></td>
					<td class="segtd"><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})"></td>
				</tr>
			</table>
			<div class="h bp" id="seg${i}len"></div>
		<i class="icons e-icon pwr ${powered[i] ? "act" : ""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
			<div class="sliderwrap il sws">
				<input id="seg${i}bri" class="noslide sis" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
				<div class="sliderdisplay"></div>
			</div>
				<i class="icons e-icon cnf" id="segc${i}" onclick="setSeg(${i})">&#xe390;</i>
				<i class="icons e-icon del" id="segd${i}" onclick="delSeg(${i})">&#xe037;</i>
				<label class="check revchkl">
					Reverse direction
					<input type="checkbox" id="seg${i}rev" onchange="setRev(${i})" ${inst.rev ? "checked" : ""}>
					<span class="checkmark schk"></span>
				</label>
				<label class="check revchkl">
					Mirror effect
					<input type="checkbox" id="seg${i}mi" onchange="setMi(${i})" ${inst.mi ? "checked" : ""}>
					<span class="checkmark schk"></span>
				</label>
			</div>
		</div><br>`;
    }

    d.getElementById('segcont').innerHTML = cn;
    if (lowestUnused >= maxSeg) {
        d.getElementById('segutil').innerHTML = '<span class="h">Maximum number of segments reached.</span>';
        noNewSegs = true;
    } else if (noNewSegs) {
        resetUtil();
        noNewSegs = false;
    }
    for (i = 0; i <= lSeg; i++) {
        updateLen(i);
        updateTrail(d.getElementById(`seg${i}bri`));
        if (segCount < 2) d.getElementById(`segd${lSeg}`).style.display = "none";
    }
    d.getElementById('rsbtn').style.display = (segCount > 1) ? "inline" : "none";
}

function updateTrail(e, slidercol) {
    if (e == null) return;
    var progress = e.value * 100 / 255;
    progress = parseInt(progress);
    var scol;
    switch (slidercol) {
        case 1: scol = "#f00"; break;
        case 2: scol = "#0f0"; break;
        case 3: scol = "#00f"; break;
        default: scol = "var(--c-f)";
    }
    var val = `linear-gradient(90deg, ${scol} ${progress}%, var(--c-4) ${progress}%)`;
    e.parentNode.getElementsByClassName('sliderdisplay')[0].style.background = val;
}

function updateLen(s) {
    if (!d.getElementById(`seg${s}s`)) return;
    var start = parseInt(d.getElementById(`seg${s}s`).value);
    var stop = parseInt(d.getElementById(`seg${s}e`).value);
    var len = stop - start;
    var out = "(delete)"
    if (len > 1) {
        out = `${len} LEDs`;
    } else if (len == 1) {
        out = "1 LED"
    }

    if (d.getElementById(`seg${s}grp`) != null) {
        var grp = parseInt(d.getElementById(`seg${s}grp`).value);
        var spc = parseInt(d.getElementById(`seg${s}spc`).value);
        if (grp == 0) grp = 1;
        var virt = Math.ceil(len / (grp + spc));
        if (!isNaN(virt) && (grp > 1 || spc > 0)) out += ` (${virt} virtual)`;
    }

    d.getElementById(`seg${s}len`).innerHTML = out;
}

function updateUI() {
    d.getElementById('buttonPower').className = (isOn) ? "active" : "";
    d.getElementById('buttonNl').className = (nlA) ? "active" : "";
    d.getElementById('buttonSync').className = (syncSend) ? "active" : "";

    d.getElementById('fxb' + selectedFx).style.backgroundColor = "var(--c-6)";
    updateTrail(d.getElementById('sliderBri'));
    updateTrail(d.getElementById('sliderSpeed'));
    updateTrail(d.getElementById('sliderIntensity'));
    updateTrail(d.getElementById('sliderW'));
    if (isRgbw) d.getElementById('wwrap').style.display = "block";

    var btns = document.getElementsByClassName("psts");
    for (i = 0; i < btns.length; i++) {
        btns[i].className = btns[i].className.replace(" active", "");
        if ((savedPresets >> i) & 0x01) btns[i].className += " stored";
    }
    if (currentPreset > 0 && currentPreset <= btns.length) btns[currentPreset - 1].className += " active";

    spal = d.getElementById("selectPalette");
    spal.style.backgroundColor = (spal.selectedIndex > 0) ? "var(--c-6)" : "var(--c-3)";
    updateHex();
    updateRgb();
}

function displayRover(i, s) {
    d.getElementById('rover').style.transform = (i.live && s.lor == 0) ? "translateY(0px)" : "translateY(100%)";
    var sour = i.lip ? i.lip : ""; if (sour.length > 2) sour = " from " + sour;
    d.getElementById('lv').innerHTML = `WLED is receiving live ${i.lm} data${sour}`;
    d.getElementById('roverstar').style.display = (i.live && s.lor) ? "block" : "none";
}

function compare(a, b) {
    if (a.name < b.name) {
        return -1;
    }
    return 1;
}
var jsonTimeout;
var reqC = 0;
function requestJson(command, rinfo = true, verbose = true) {
    d.getElementById('connind').style.backgroundColor = "#a90";
    lastUpdate = new Date();
    if (!jsonTimeout) jsonTimeout = setTimeout(showErrorToast, 3000);
    var req = null;
    e1 = d.getElementById('fxlist');
    e2 = d.getElementById('selectPalette');

    url = rinfo ? '/json/si' : (command ? '/json/state' : '/json');
    if (loc) {
        url = `http://${locip}${url}`;
    }

    type = command ? 'post' : 'get';
    if (command) {
        command.v = verbose;
        req = JSON.stringify(command);
    }
    fetch
        (url, {
            method: type,
            headers: {
                "Content-type": "application/json; charset=UTF-8"
            },
            body: req
        })
        .then(res => {
            if (!res.ok) {
                showErrorToast();
            }
            return res.json();
        })
        .then(json => {
            clearTimeout(jsonTimeout);
            jsonTimeout = null;
            clearErrorToast();
            d.getElementById('connind').style.backgroundColor = "#070";
            if (!json) showToast('Empty response', true);
            if (json.success) return;
            var s = json;
            if (!command || rinfo) {
                if (!rinfo) {
                    var x = '', y = '<option value="0">Default</option>';
                    json.effects.shift(); //remove solid
                    for (i in json.effects) json.effects[i] = { id: parseInt(i) + 1, name: json.effects[i] };
                    json.effects.sort(compare);
                    for (i in json.effects) {
                        x += `<button class="btn${(i == 0) ? " first" : ""}" id="fxb${json.effects[i].id}" onclick="setX(${json.effects[i].id});">${json.effects[i].name}</button><br>`;
                    }

                    json.palettes.shift(); //remove default
                    for (i in json.palettes) json.palettes[i] = { "id": parseInt(i) + 1, "name": json.palettes[i] };
                    json.palettes.sort(compare);
                    for (i in json.palettes) {
                        y += `<option value="${json.palettes[i].id}">${json.palettes[i].name}</option>`;
                    }
                    e1.innerHTML = x; e2.innerHTML = y;
                }

                var info = json.info;
                var name = info.name;
                d.getElementById('namelabel').innerHTML = name;
                if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)";
                if (info.live) name = "(Live) " + name;
                if (loc) name = "(L) " + name;
                d.title = name;
                isRgbw = info.leds.wv;
                ledCount = info.leds.count;
                syncTglRecv = info.str;
                maxSeg = info.leds.maxseg;
                lastinfo = info;
                if (isInfo) populateInfo(info);
                s = json.state;
                displayRover(info, s);
            }
            isOn = s.on;
            d.getElementById('sliderBri').value = s.bri;
            nlA = s.nl.on;
            nlDur = s.nl.dur;
            nlTar = s.nl.tbri;
            nlFade = s.nl.fade;
            syncSend = s.udpn.send;
            savedPresets = s.pss;
            currentPreset = s.ps;
            d.getElementById('cyToggle').checked = (s.pl < 0) ? false : true;
            d.getElementById('cycs').value = s.ccnf.min;
            d.getElementById('cyce').value = s.ccnf.max;
            d.getElementById('cyct').value = s.ccnf.time / 10;
            d.getElementById('cyctt').value = s.transition / 10;

            var selc = 0; var ind = 0;
            populateSegments(s);
            for (i in s.seg) {
                if (s.seg[i].sel) { selc = ind; break; } ind++;
            }
            var i = s.seg[selc];
            if (!i) {
                showToast('No Segments!', true);
                updateUI();
                return;
            }
            var cd = d.getElementById('csl').children;
            for (e = 2; e >= 0; e--) {
                cd[e].style.backgroundColor = "rgb(" + i.col[e][0] + "," + i.col[e][1] + "," + i.col[e][2] + ")";
                if (isRgbw) whites[e] = parseInt(i.col[e][3]);
                selectSlot(csel);
            }
            d.getElementById('sliderSpeed').value = whites[csel];

            d.getElementById('sliderSpeed').value = i.sx;
            d.getElementById('sliderIntensity').value = i.ix;

            d.getElementById('fxb' + selectedFx).style.backgroundColor = "var(--c-3)";
            selectedFx = i.fx;
            e2.value = i.pal;
            if (!command) d.getElementById('Effects').scrollTop = d.getElementById('fxb' + selectedFx).offsetTop - d.getElementById('Effects').clientHeight / 1.8;

            if (s.error) showToast('WLED error ' + s.error, true);
            updateUI();
        })
        .catch(function (error) {
            showToast(error, true);
            console.log(error);
        })
}

function togglePower() {
    isOn = !isOn;
    var obj = { "on": isOn };
    obj.transition = parseInt(d.getElementById('cyctt').value * 10);
    requestJson(obj);
}

function toggleNl() {
    nlA = !nlA;
    if (nlA) {
        showToast(`Timer active. Your light will turn ${nlTar > 0 ? "on" : "off"} ${nlFade ? "over" : "after"} ${nlDur} minutes.`);
    } else {
        showToast('Timer deactivated.');
    }
    var obj = { "nl": { "on": nlA } };
    requestJson(obj);
}

function toggleSync() {
    syncSend = !syncSend;
    if (syncSend) {
        showToast('Other lights in the network will now sync to this one.');
    } else {
        showToast('This light and other lights in the network will no longer sync.');
    }
    var obj = { "udpn": { "send": syncSend } };
    if (syncTglRecv) obj.udpn.recv = syncSend;
    requestJson(obj);
}

function toggleLiveview() {
    isLv = !isLv;
    d.getElementById('liveview').style.display = (isLv) ? "block" : "none";
    var url = loc ? `http://${locip}/liveview` : "/liveview";
    d.getElementById('liveview').src = (isLv) ? url : "about:blank";
    d.getElementById('buttonSr').className = (isLv) ? "active" : "";
    size();
}

function toggleInfo() {
    isInfo = !isInfo;
    if (isInfo) populateInfo(lastinfo);
    d.getElementById('info').style.transform = (isInfo) ? "translateY(0px)" : "translateY(100%)";
    d.getElementById('buttonI').className = (isInfo) ? "active" : "";
}

function makeSeg() {
    var ns = 0;
    if (lowestUnused > 0) {
        var pend = d.getElementById(`seg${lowestUnused - 1}e`).value;
        if (pend < ledCount) ns = pend;
    }
    var cn = `<div class="seg">
			<div class="segname newseg">
				New segment ${lowestUnused}
			</div>
			<br>
			<div class="segin expanded">
				<table class="segt">
					<tr>
						<td class="segtd">Start LED</td>
						<td class="segtd">Stop LED</td>
					</tr>
					<tr>
						<td class="segtd"><input class="noslide segn" id="seg${lowestUnused}s" type="number" min="0" max="${ledCount - 1}" value="${ns}" oninput="updateLen(${lowestUnused})"></td>
						<td class="segtd"><input class="noslide segn" id="seg${lowestUnused}e" type="number" min="0" max="${ledCount}" value="${ledCount}" oninput="updateLen(${lowestUnused})"></td>
					</tr>
				</table>
				<div class="h" id="seg${lowestUnused}len">${ledCount - ns} LEDs</div>
				<i class="icons e-icon cnf half" id="segc${lowestUnused}" onclick="setSeg(${lowestUnused}); resetUtil();">&#xe390;</i>
			</div>
		</div>`;
    d.getElementById('segutil').innerHTML = cn;
}

function resetUtil() {
    var cn = `<button class="btn btn-s btn-i rect" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>Add segment</button><br>`;
    d.getElementById('segutil').innerHTML = cn;
}

function selSegEx(s) {
    var obj = { "seg": [] };
    for (i = 0; i <= lSeg; i++) {
        obj.seg.push({ "sel": (i == s) ? true : false });
    }
    requestJson(obj);
}

function selSeg(s) {
    var sel = d.getElementById(`seg${s}sel`).checked;
    var obj = { "seg": { "id": s, "sel": sel } };
    requestJson(obj, false);
}

function setSeg(s) {
    var start = parseInt(d.getElementById(`seg${s}s`).value);
    var stop = parseInt(d.getElementById(`seg${s}e`).value);
    if (stop <= start) { delSeg(s); return; };
    var obj = { "seg": { "id": s, "start": start, "stop": stop } };
    if (d.getElementById(`seg${s}grp`)) {
        var grp = parseInt(d.getElementById(`seg${s}grp`).value);
        var spc = parseInt(d.getElementById(`seg${s}spc`).value);
        obj.seg.grp = grp;
        obj.seg.spc = spc;
    }
    requestJson(obj);
}

function delSeg(s) {
    if (segCount < 2) {
        showToast("You need to have multiple segments to delete one!");
        return;
    }
    expanded[s] = false;
    segCount--;
    var obj = { "seg": { "id": s, "stop": 0 } };
    requestJson(obj, false);
}

function setRev(s) {
    var rev = d.getElementById(`seg${s}rev`).checked;
    var obj = { "seg": { "id": s, "rev": rev } };
    requestJson(obj, false);
}

function setMi(s) {
    var mi = d.getElementById(`seg${s}mi`).checked;
    var obj = { "seg": { "id": s, "mi": mi } };
    requestJson(obj, false);
}

function setSegPwr(s) {
    var obj = { "seg": { "id": s, "on": !powered[s] } };
    requestJson(obj);
}

function setSegBri(s) {
    var obj = { "seg": { "id": s, "bri": parseInt(d.getElementById(`seg${s}bri`).value) } };
    requestJson(obj);
}

function setX(ind) {
    var obj = { "seg": { "fx": parseInt(ind) } };
    requestJson(obj);
}

function setPalette() {
    var obj = { "seg": { "pal": parseInt(d.getElementById('selectPalette').value) } };
    requestJson(obj);
}

function setBri() {
    var obj = { "bri": parseInt(d.getElementById('sliderBri').value) };
    obj.transition = parseInt(d.getElementById('cyctt').value * 10);
    requestJson(obj);
}

function setSpeed() {
    var obj = { "seg": { "sx": parseInt(d.getElementById('sliderSpeed').value) } };
    requestJson(obj, false);
}

function setIntensity() {
    var obj = { "seg": { "ix": parseInt(d.getElementById('sliderIntensity').value) } };
    requestJson(obj, false);
}

function setLor(i) {
    var obj = { "lor": i };
    requestJson(obj);
}

function toggleCY() {
    var obj = { "pl": -1 };
    if (d.getElementById('cyToggle').checked) {
        obj = { "pl": 0, "ccnf": { "min": parseInt(d.getElementById('cycs').value), "max": parseInt(d.getElementById('cyce').value), "time": parseInt(d.getElementById('cyct').value * 10) } };
        obj.transition = parseInt(d.getElementById('cyctt').value * 10);
    }

    requestJson(obj);
}

function togglePS() {
    ps = !ps;

    var btns = document.getElementsByClassName("psts");
    for (i = 0; i < btns.length; i++) {
        if (ps) {
            btns[i].className += " saving";
        } else {
            btns[i].className = btns[i].className.replace(" saving", "");
        }
    }

    d.getElementById("psLabel").innerHTML = (ps) ? "Save to slot" : "Load from slot";
}

function setPreset(i) {
    var obj = { "ps": i }
    if ((savedPresets >> (i - 1)) & 0x01) {
        showToast("Loading config from slot " + i + ".");
    } else {
        showToast("Slot " + i + " is empty! Use saving mode to save the current config to it.");
    }
    if (ps) {
        obj = { "psave": i };
        showToast("Saving config to slot " + i + ".");
    }
    requestJson(obj);
}

function selectSlot(b) {
    csel = b;
    var cd = d.getElementById('csl').children;
    for (i = 0; i < cd.length; i++) {
        cd[i].style.border = "2px solid white";
        cd[i].style.margin = "5px";
        cd[i].style.width = "42px";
    }
    cd[csel].style.border = "5px solid white";
    cd[csel].style.margin = "2px";
    cd[csel].style.width = "50px";
    cpick.color.set(cd[csel].style.backgroundColor);
    d.getElementById('sliderW').value = whites[csel];
    updateTrail(d.getElementById('sliderW'));
    updateHex();
    updateRgb();
}

var lasth = 0;
function pC(col) {
    if (col == "rnd") {
        col = { h: 0, s: 0, v: 100 };
        col.s = Math.floor((Math.random() * 50) + 50);
        do {
            col.h = Math.floor(Math.random() * 360);
        } while (Math.abs(col.h - lasth) < 50);
        lasth = col.h;
    }
    cpick.color.set(col);
    setColor(0);
}

function updateRgb() {
    var col = cpick.color.rgb;
    var s = d.getElementById('sliderR');
    s.value = col.r; updateTrail(s, 1);
    s = d.getElementById('sliderG');
    s.value = col.g; updateTrail(s, 2);
    s = d.getElementById('sliderB');
    s.value = col.b; updateTrail(s, 3);
}

function updateHex() {
    var str = cpick.color.hexString;
    str = str.substring(1);
    var w = whites[csel];
    if (w > 0) str += w.toString(16);
    d.getElementById('hexc').value = str;
    d.getElementById('hexcnf').style.backgroundColor = "var(--c-3)";
}

function hexEnter() {
    d.getElementById('hexcnf').style.backgroundColor = "var(--c-6)";
    if (event.keyCode == 13) fromHex();
}

function fromHex() {
    var str = d.getElementById('hexc').value;
    console.log(str);
    whites[csel] = parseInt(str.substring(6), 16);
    try {
        cpick.color.set("#" + str.substring(0, 6));
    } catch (e) {
        cpick.color.set("#ffaa00");
    }
    if (isNaN(whites[csel])) whites[csel] = 0;
    setColor(2);
}

function fromRgb() {
    var r = d.getElementById('sliderR').value;
    var g = d.getElementById('sliderG').value;
    var b = d.getElementById('sliderB').value;
    cpick.color.set(`rgb(${r},${g},${b})`);
    setColor(0);
}

function setColor(sr) {
    var cd = d.getElementById('csl').children;
    if (sr == 1 && cd[csel].style.backgroundColor == 'rgb(0, 0, 0)') cpick.color.setChannel('hsv', 'v', 100);
    cd[csel].style.backgroundColor = cpick.color.rgbString;
    if (sr != 2) whites[csel] = d.getElementById('sliderW').value;
    var col = cpick.color.rgb;
    var obj = { "seg": { "col": [[col.r, col.g, col.b, whites[csel]], [], []] } };
    if (csel == 1) {
        obj = { "seg": { "col": [[], [col.r, col.g, col.b, whites[csel]], []] } };
    } else if (csel == 2) {
        obj = { "seg": { "col": [[], [], [col.r, col.g, col.b, whites[csel]]] } };
    }
    updateHex();
    updateRgb();
    obj.transition = parseInt(d.getElementById('cyctt').value * 10);
    requestJson(obj);
}

var hc = 0;
setInterval(function () {
    if (!isInfo) return; hc += 18; if (hc > 300) hc = 0; if (hc > 200) hc = 306; if (hc == 144) hc += 36; if (hc == 108) hc += 18;
    d.getElementById('heart').style.color = `hsl(${hc}, 100%, 50%)`
}, 910);

function openGH() {
    window.open("https://github.com/Aircoookie/WLED/wiki");
}

var cnfr = false;
function cnfReset() {
    if (!cnfr) {
        var bt = d.getElementById('resetbtn');
        bt.style.color = "#f00";
        bt.innerHTML = "Confirm Reboot";
        cnfr = true; return;
    }
    window.location.href = "/reset";
}

var cnfrS = false;
function rSegs() {
    var bt = d.getElementById('rsbtn');
    if (!cnfrS) {
        bt.style.color = "#f00";
        bt.innerHTML = "Confirm reset";
        cnfrS = true; return;
    }
    cnfrS = false;
    bt.style.color = "#fff";
    bt.innerHTML = "Reset segments";
    var obj = { "seg": [{ "start": 0, "stop": ledCount, "sel": true }] };
    for (i = 1; i <= lSeg; i++) {
        obj.seg.push({ "stop": 0 });
    }
    requestJson(obj);
}

function expand(i) {
    expanded[i] = !expanded[i];
    d.getElementById('seg' + i).style.display = (expanded[i]) ? "block" : "none";
    d.getElementById('sege' + i).style.transform = (expanded[i]) ? "rotate(180deg)" : "rotate(0deg)"
}

function unfocusSliders() {
    d.getElementById("sliderBri").blur();
    d.getElementById("sliderSpeed").blur();
    d.getElementById("sliderIntensity").blur();
}

//sliding UI
const _C = document.querySelector('.container'), N = 4;

let iSlide = 0, x0 = null, y0 = null, scrollS = 0, locked = false, w;

function unify(e) { return e.changedTouches ? e.changedTouches[0] : e }

function lock(e) {
    if (pcMode) return;
    var l = e.target.classList;
    if (l.contains('noslide') || l.contains('iro__wheel__saturation') || l.contains('iro__slider__value') || l.contains('iro__slider')) return;
    x0 = unify(e).clientX;
    y0 = unify(e).clientY;
    scrollS = d.getElementsByClassName("tabcontent")[iSlide].scrollTop;

    _C.classList.toggle('smooth', !(locked = true))
}

function drag(e) {
    if (!locked || pcMode) return;
    if (d.getElementsByClassName("tabcontent")[iSlide].scrollTop != scrollS) {
        move(e); return;
    }

    _C.style.setProperty('--tx', `${Math.round(unify(e).clientX - x0)}px`)
}

function move(e) {
    if (!locked || pcMode) return;
    var dx = unify(e).clientX - x0, s = Math.sign(dx),
        f = +(s * dx / w).toFixed(2);

    if ((iSlide > 0 || s < 0) && (iSlide < N - 1 || s > 0) && f > .12) {
        _C.style.setProperty('--i', iSlide -= s);
        f = 1 - f;
        updateTablinks(iSlide);
    }

    _C.style.setProperty('--tx', '0px');
    _C.style.setProperty('--f', f);
    _C.classList.toggle('smooth', !(locked = false));
    x0 = null
}

function size() {
    w = window.innerWidth;
    var h = d.getElementById('top').clientHeight;
    sCol('--th', h + "px");
    sCol('--bh', d.getElementById('bot').clientHeight + "px");
    if (isLv) h -= 4;
    sCol('--tp', h + "px");
    togglePcMode();
}

function togglePcMode(fromB = false) {
    if (fromB) {
        pcModeA = !pcModeA;
        localStorage.setItem('pcm', pcModeA);
        pcMode = pcModeA;
    }
    if (w < 1250 && !pcMode) return;
    if (!fromB && ((w < 1250 && lastw < 1250) || (w >= 1250 && lastw >= 1250))) return;
    openTab(0, true);
    if (w < 1250) { pcMode = false; }
    else if (pcModeA && !fromB) pcMode = pcModeA;
    updateTablinks(0);
    d.getElementById('buttonPcm').className = (pcMode) ? "active" : "";
    d.getElementById('bot').style.height = (pcMode && !cfg.comp.pcmbot) ? "0" : "auto";
    sCol('--bh', d.getElementById('bot').clientHeight + "px");
    if (pcMode) {
        _C.style.width = '100%';
    } else {
        _C.style.width = '400%';
    }
    lastw = w;
}

function isObject(item) {
    return (item && typeof item === 'object' && !Array.isArray(item));
}

function mergeDeep(target, ...sources) {
    if (!sources.length) return target;
    const source = sources.shift();

    if (isObject(target) && isObject(source)) {
        for (const key in source) {
            if (isObject(source[key])) {
                if (!target[key]) Object.assign(target, { [key]: {} });
                mergeDeep(target[key], source[key]);
            } else {
                Object.assign(target, { [key]: source[key] });
            }
        }
    }
    return mergeDeep(target, ...sources);
}

size();
_C.style.setProperty('--n', N);

window.addEventListener('resize', size, false);

_C.addEventListener('mousedown', lock, false);
_C.addEventListener('touchstart', lock, false);

_C.addEventListener('mousemove', drag, false);
_C.addEventListener('touchmove', drag, false);

_C.addEventListener('mouseout', move, false);
_C.addEventListener('mouseup', move, false);
_C.addEventListener('touchend', move, false);