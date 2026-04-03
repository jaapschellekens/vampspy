RM = rm -f

def:
	@echo targets are:
	@echo output -- update vamps run
	@echo clean -- cleans up rubbish

clean:
	${RM} *.set *_stop *.log gr.csh *.tt *~ *.out example1.rep

output: example1.out
	vamps example1.inp

