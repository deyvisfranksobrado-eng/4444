const phone='51930663790';
function whatsapp(message){return `https://wa.me/${phone}?text=${encodeURIComponent(message)}`;}
document.querySelectorAll('[data-wa]').forEach(link=>{
  const context=link.dataset.wa||'tus asesorías';
  link.href=whatsapp(`Hola Deyvis, vengo de tu página web y quisiera más información sobre ${context}. ¿Podrías contarme sobre la disponibilidad y los costos?`);
  link.target='_blank';link.rel='noopener noreferrer';
});
const form=document.querySelector('#contact-form');
if(form){
const requested=new URLSearchParams(window.location.search).get('materia');
if(requested){const select=form.querySelector('#materia');const option=[...select.options].find(item=>item.value===requested);if(option)select.value=option.value;}
form.addEventListener('submit',event=>{
  event.preventDefault();
  if(!form.reportValidity())return;
  const data=new FormData(form);
  const name=String(data.get('nombre')).trim();
  const subject=String(data.get('materia'));
  const plan=String(data.get('plan'));
  const note=String(data.get('mensaje')).trim();
  const message=`Hola Deyvis, vengo de tu página web. Soy ${name} y quisiera información sobre asesorías de ${subject}. Me interesa el plan ${plan}.${note?` Mi consulta: ${note}`:''} ¿Podrías contarme sobre la disponibilidad y los costos?`;
  window.open(whatsapp(message),'_blank','noopener,noreferrer');
});}
