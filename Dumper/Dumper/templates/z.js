async function copyTextToClipboard(text) {
	try {
		await navigator.clipboard.writeText(text);
	} catch (err) {
		console.error("failed to copy text: ", err);
	}
}

window.addEventListener("load", () => {
	// Check system preference and switch to that theme.
	let html_tag = document.getElementsByTagName("html")[0];
	if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
		html_tag.setAttribute("data-theme", "dark");
	} else {
		html_tag.setAttribute("data-theme", "light");
	}

	// Setup theme switching.
	document.getElementById("dark-btn").addEventListener("click", (e) => {
		e.preventDefault();
		html_tag.setAttribute("data-theme", "dark");
	});
	document.getElementById("light-btn").addEventListener("click", (e) => {
		e.preventDefault();
		html_tag.setAttribute("data-theme", "light");
	});

	// Allow the previous page to pass the name to this page.
	const searchParams = new URLSearchParams(window.location.search);
	if (searchParams.has("n")) {
		document.getElementById("vn").innerHTML = sp.get("n");
	}

	// Allow copying a method's name.
	var methodElements = document.querySelectorAll("#method div ul li");
	methodElements.forEach((element) => {
		element.addEventListener("click", (e) => {
			// Parse for the method name.
			const tokens = element.innerText.split("'");
			if (tokens.length >= 2) {
				copyTextToClipboard(tokens[1]);
			}
		});
	});
});


