# vychozi obrazky
#SRC=neco.fig
# pozadovany format pro LaTeX
DEST=logo.pdf

.SUFFIXES: .eps .pdf

all: $(DEST)

.eps.pdf:
	epstopdf --outfile=$@ $<
