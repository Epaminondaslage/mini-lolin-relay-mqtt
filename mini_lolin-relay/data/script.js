function atualizarEstado() {
  fetch('/status.json')
    .then(response => response.json())
    .then(data => {
      document.getElementById('estado').innerText = 'Estado atual: ' + data.rele;
      document.getElementById('ip').innerText = data.ip;
    });
}

function alternarRele() {
  fetch('/toggle', { method: 'POST' }).then(() => atualizarEstado());
}

setInterval(atualizarEstado, 5000);
window.onload = atualizarEstado;
