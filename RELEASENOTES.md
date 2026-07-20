### 0.0.1

feature: add attributed text rendering and expose line-gap metrics

* added NVGtextAttribute and nvgAttributedText() for per-run color/font/size/y-offset attributed text (supports ClearType background hints and emoji)
* added nvgTextMetricsWithGap() to return ascender/descender/lineh plus lineGap for precise vertical layout
* extended fontstash with fonsVertMetrics4() and lineGap storage; added demos (attributed-text, cleartype) and RLGL blend diagnostic doc












  redraw at 60fps)

  bypassing rlgl's batch-aware blend API entirely
  an error if called while an external renderer owns GL state
