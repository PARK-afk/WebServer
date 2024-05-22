const element = document.getElementById('blinkingElement');

let isVisible = true;

function toggleVisibility() {
  isVisible = !isVisible;
  element.style.visibility = isVisible ? 'visible' : 'hidden';
}

setInterval(toggleVisibility, 500);