# Definisci le directory che contengono i sotto-makefile
SUBDIRS := "BarnesHut_serial" "mpi_BarnesHut" "mpi_parallel_nbodies" "pthread_BarnesHut" "pthread_parallel_nbodies" "serial_nbodies"

.PHONY: all clean

# Regola per compilare tutti i makefile nelle sottocartelle
all: $(SUBDIRS)

# Itera su ciascuna sottodirectory ed esegui `make` lì
$(SUBDIRS):
	$(MAKE) -C $@

# Regola per pulire tutte le sottodirectory
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done