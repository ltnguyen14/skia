const animFetch = fetch("./videoAnim.json").then(bytes => bytes.text());
const image1Fetch = fetch("./image_1.jpg").then(blob => blob.arrayBuffer());
const image2Fetch = fetch("./image_2.jpg").then(blob => blob.arrayBuffer());

Promise.all([window.LamKitInit(), animFetch, image1Fetch, image2Fetch]).then(([lamKit, animationSource, image1, image2]) => {
  const surface = lamKit.MakeCanvasSurface(document.getElementById("canvasEl"));
  const videoEl = document.getElementById('video');

  const videoCanvas = document.getElementById('videoCanvas');
  const canvasContext = videoCanvas.getContext("2d");

  let resourceProvider = lamKit.MakeResourceProvider({
    "image": image1,
  });
  let animation = lamKit.MakeAnimation(animationSource, resourceProvider);

  frame = 0;
  function drawFrame() {
    lamKit.RenderAnimation(surface, animation, frame);
    frame++;
    if (frame == 300) frame = 0;
    canvasContext.drawImage(videoEl, 0, 0);

    const videoFrame = videoCanvas.toBlob((blob) => {
      const arrBuff = blob.arrayBuffer().then(buff => {
        resourceProvider = lamKit.MakeResourceProvider({
          "image": buff,
        });
        animation = lamKit.MakeAnimation(animationSource, resourceProvider);

        requestAnimationFrame(drawFrame);
      });
    });
  }
  requestAnimationFrame(drawFrame);
});

window.CanvasKitInit().then((canvasKit) => {
  const surface = canvasKit.MakeCanvasSurface(document.getElementById("canvasKitEl"));
});
