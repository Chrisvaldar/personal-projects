//Navigation Bar
// checks the value of aria expanded on the cs-ul and changes it accordingly whether it is expanded or not
function ariaExpanded() {
	const csUL = document.querySelector('#cs-expanded');
	const csExpanded = csUL.getAttribute('aria-expanded');

	if (csExpanded === 'false') {
		csUL.setAttribute('aria-expanded', 'true');
	} else {
		csUL.setAttribute('aria-expanded', 'false');
	}
}                               

//Booking Form
const faqItems = Array.from(document.querySelectorAll('.cs-faq-item'));
        for (const item of faqItems) {
            const onClick = () => {
            item.classList.toggle('active')
        }
        item.addEventListener('click', onClick)
        }

//Gallery JavaScript (not from CodeStitch)
const galleryImages = document.querySelectorAll(".cs-image");
const lightbox = document.getElementById("lightbox");
const lightboxImg = document.getElementById("lightbox-img");

galleryImages.forEach(function (img) {
    img.addEventListener("click", function () {
        lightboxImg.src = img.src;
        lightbox.classList.add("active");
    });
});

lightbox.addEventListener("click", function () {
    lightbox.classList.remove("active");
});

flatpickr("#date-1333", {
    dateFormat: "Y-m-d",
    minDate: "today",
    disableMobile: true
});