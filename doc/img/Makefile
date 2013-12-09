# vychozi obrazky
#SRC=neco.fig
# pozadovany format pro LaTeX
#DEST=neco.eps

.SUFFIXES: .fig .eps .png .jpg

all: $(DEST)

.fig.eps:
	fig2dev -L eps $< $@

.png.eps:
	convert $< $@

.jpg.eps:
	convert $< $@
