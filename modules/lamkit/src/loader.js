const animFetch = fetch("./animation.json").then(bytes => bytes.text());

Promise.all([window.LamKitInit(), animFetch]).then(([lamKit, animationSource]) => {
  const surface = lamKit.MakeCanvasSurface(document.getElementById("canvasEl"));
  // lamKit.Paint(surface);
  const animation = lamKit.MakeAnimation(animationSource, lamKit.MakeResourceProvider());

  frame = 0;
  function drawFrame() {
    lamKit.RenderAnimation(surface, animation, frame);
    frame++;
    if (frame == 60) frame = 0;

    requestAnimationFrame(drawFrame);
  }
  requestAnimationFrame(drawFrame);
});

window.CanvasKitInit().then((canvasKit) => {
  const surface = canvasKit.MakeCanvasSurface(document.getElementById("canvasKitEl"));
});
