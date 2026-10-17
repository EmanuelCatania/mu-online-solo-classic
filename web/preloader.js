/**
 * ============================================================================
 * MU Online Solo Classic - Preloader & Atmospheric Particle System
 * ============================================================================
 */

(function () {
  'use strict';

  // ── 1. Dynamic Background Particle Canvas ─────────────────────────────────

  var particleCanvas = document.getElementById('muParticleCanvas');
  var pCtx = particleCanvas ? particleCanvas.getContext('2d') : null;
  var particleAnimId = null;
  var pWidth = 0;
  var pHeight = 0;
  var particles = [];
  var PARTICLE_COUNT = 95;

  function resizeParticles() {
    if (!particleCanvas) return;
    pWidth = particleCanvas.width = window.innerWidth;
    pHeight = particleCanvas.height = window.innerHeight;
  }

  function initParticles() {
    if (!particleCanvas || !pCtx) return;
    resizeParticles();
    window.addEventListener('resize', resizeParticles);

    for (var i = 0; i < PARTICLE_COUNT; i++) {
      particles.push({
        x: Math.random() * (pWidth || 800),
        y: Math.random() * (pHeight || 600),
        radius: Math.random() * 1.6 + 0.5,
        vx: (Math.random() - 0.5) * 0.28,
        vy: -(Math.random() * 0.42 + 0.18), // gentle upward drift
        alpha: Math.random() * 0.5 + 0.15,
        baseAlpha: Math.random() * 0.45 + 0.15,
        pulseSpeed: Math.random() * 0.02 + 0.008,
        pulseAngle: Math.random() * Math.PI * 2,
        // Modern palette: cold platinum / silver + subtle ruby energy dust
        color: Math.random() > 0.22 ? '215, 225, 240' : '239, 68, 68'
      });
    }

    function renderFrame() {
      pCtx.clearRect(0, 0, pWidth, pHeight);
      for (var i = 0; i < particles.length; i++) {
        var p = particles[i];
        p.x += p.vx;
        p.y += p.vy;
        p.pulseAngle += p.pulseSpeed;
        p.alpha = p.baseAlpha + Math.sin(p.pulseAngle) * 0.16;

        if (p.y < -10) {
          p.y = pHeight + 10;
          p.x = Math.random() * pWidth;
        }
        if (p.x < -10) p.x = pWidth + 10;
        if (p.x > pWidth + 10) p.x = -10;

        pCtx.beginPath();
        pCtx.arc(p.x, p.y, p.radius, 0, Math.PI * 2);
        pCtx.fillStyle = 'rgba(' + p.color + ',' + Math.max(0, p.alpha.toFixed(2)) + ')';
        if (p.radius > 1.2) {
          pCtx.shadowBlur = 6;
          pCtx.shadowColor = 'rgba(' + p.color + ', 0.5)';
        } else {
          pCtx.shadowBlur = 0;
        }
        pCtx.fill();
      }
      particleAnimId = requestAnimationFrame(renderFrame);
    }

    particleAnimId = requestAnimationFrame(renderFrame);
  }

  function stopParticles() {
    if (particleAnimId) {
      cancelAnimationFrame(particleAnimId);
      particleAnimId = null;
    }
    window.removeEventListener('resize', resizeParticles);
    if (pCtx && particleCanvas) {
      pCtx.clearRect(0, 0, pWidth, pHeight);
    }
    particles = [];
    if (particleCanvas && particleCanvas.parentNode) {
      particleCanvas.remove();
      particleCanvas = null;
      pCtx = null;
    }
  }

  initParticles();

  // ── 2. Preloader Lifecycle & Status Management ────────────────────────────

  var preloader = document.getElementById('muPreloader');
  var barFill = document.getElementById('muBarFill');
  var statusMsg = document.getElementById('muStatusMsg');
  var statusDetail = document.getElementById('muStatusDetail');
  var tipMsg = document.getElementById('muTipText');

  var muTips = [
    "Use hotkeys Q, W, E to quickly drink Health and Mana potions.",
    "Press I or V to open Inventory, and C to view Character stats.",
    "All characters, items, and settings are saved automatically in your browser.",
    "Press Esc to pause, change options, or safely save and exit the game.",
    "Jewel of Bless upgrades your weapons and armor up to +6 with a 100% success rate.",
    "Hunt Spiders and Budge Dragons outside Lorencia gates for quick early levels.",
    "Type /post in chat to broadcast a message across the world."
  ];

  var tipIndex = 0;
  var tipInterval = setInterval(function () {
    if (!tipMsg) return;
    tipMsg.style.opacity = '0';
    setTimeout(function () {
      tipIndex = (tipIndex + 1) % muTips.length;
      tipMsg.textContent = muTips[tipIndex];
      tipMsg.style.opacity = '1';
    }, 300);
  }, 4200);

  function setPreloaderProgress(percent, msg, detail) {
    if (!barFill) return;
    var p = Math.max(2, Math.min(100, percent));
    barFill.style.width = p + '%';
    if (msg && statusMsg) statusMsg.textContent = msg;
    if (detail && statusDetail) statusDetail.textContent = detail;
  }

  function finishPreloader() {
    if (!preloader || preloader._finished) return;
    preloader._finished = true;

    // Immediately stop and unmount all background particles so zero GPU/CPU is spent during game
    stopParticles();

    setPreloaderProgress(100, "Entering Lorencia...", "100%");
    if (tipInterval) {
      clearInterval(tipInterval);
      tipInterval = 0;
    }

    setTimeout(function () {
      preloader.classList.add('fade-out');
      setTimeout(function () {
        preloader.style.display = 'none';
      }, 650);
    }, 350);
  }

  // Global Exports for Module and Application Controller
  window.muPreloader = {
    setProgress: setPreloaderProgress,
    finish: finishPreloader,
    stopParticles: stopParticles
  };

  window.setPreloaderProgress = setPreloaderProgress;
  window.finishPreloader = finishPreloader;
  window.stopPreloaderParticles = stopParticles;
})();
