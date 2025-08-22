const dt = "data-theme";
let w = window;
let d = document;
w.addEventListener("load", () => {
	let h = document.getElementsByTagName("html")[0];
	if (w.matchMedia && w.matchMedia('(prefers-color-scheme: dark)').matches) {
		h.setAttribute(dt, "dark");
	} else {
		h.setAttribute(dt, "light");
	}
	document.getElementById("dark-btn").addEventListener("click", (e) => {
		e.preventDefault();
		h.setAttribute(dt, "dark");
	});
	document.getElementById("light-btn").addEventListener("click", (e) => {
		e.preventDefault();
		h.setAttribute(dt, "light");
	});
	const sp = new URLSearchParams(w.location.search);
	if (sp.has("n")) {
		let e = d.getElementById("vn");
		e.innerHTML = sp.get("n");
	}
});
