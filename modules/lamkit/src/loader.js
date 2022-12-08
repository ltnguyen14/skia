window.LamKitInit().then((lamKit) => {
  const ctx = lamKit.MakeCanvasSurface(document.getElementById("canvasEl"));
  console.log(ctx);
});
