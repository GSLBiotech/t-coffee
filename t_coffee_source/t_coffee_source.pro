######################################################################
# Automatically generated by qmake (3.1) Thu Jun 14 14:04:38 2018
######################################################################

TEMPLATE = app
TARGET = t_coffee_source
INCLUDEPATH += .

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += coffee_defines.h \
           define_header.h \
           dev1_lib_header.h \
           dp_lib_header.h \
           fastal_lib_header.h \
           ga_lib_header.h \
           io_lib_header.h \
           km_coffee_header.h \
           matrices.h \
           perl_header_lib.h \
           programmes_define.h \
           util_lib_header.h \
           data_headers/perl_header_lib.h \
           data_headers/programmes_define.h \
           dev_lib/dev1.h \
           dp_lib/CUSTOM_evaluate_for_struc.h \
           dp_lib/evaluate.h \
           dp_lib/evaluate_dirichlet.h \
           dp_lib/evaluate_for_domain.h \
           dp_lib/evaluate_for_struc.h \
           dp_lib/fasta_dp.h \
           dp_lib/fsa_dp.h \
           dp_lib/pavie.h \
           dp_lib/pavie_dp.h \
           dp_lib/showpair.h \
           dp_lib/util_domain_dp.h \
           dp_lib/util_domain_dp_drivers.h \
           dp_lib/util_dp.h \
           dp_lib/util_dp_cdna.h \
           dp_lib/util_dp_cdna_fasta_nw.h \
           dp_lib/util_dp_cdna_gotoh_nw.h \
           dp_lib/util_dp_clean_maln.h \
           dp_lib/util_dp_drivers.h \
           dp_lib/util_dp_est.h \
           dp_lib/util_dp_fasta_nw.h \
           dp_lib/util_dp_fasta_sw.h \
           dp_lib/util_dp_generic_fasta_nw.c.h \
           dp_lib/util_dp_generic_fasta_nw.h \
           dp_lib/util_dp_gotoh_nw.h \
           dp_lib/util_dp_gotoh_sw.h \
           dp_lib/util_dp_linear_nw.h \
           dp_lib/util_dp_mm_nw.h \
           dp_lib/util_dp_sim.h \
           dp_lib/util_dp_ssec_pwaln.h \
           dp_lib/util_dp_suboptimal_nw.h \
           dp_lib/util_graph_maln.h \
           fastal/diagonal.h \
           fastal/fastal.h \
           fastal/fastal_opt_parsing.h \
           fastal/iteration.h \
           fastal/parttree.h \
           fastal/scoring.h \
           fastal/tree.h \
           io_lib/io_func.h \
           io_lib/io_structures.h \
           io_lib/pb_util_read_seq_util.h \
           io_lib/pb_util_read_sequence.h \
           io_lib/pb_util_read_sequence2.h \
           io_lib/tree_util.h \
           km_coffee/classes.h \
           km_coffee/km_coffee.h \
           km_coffee/km_util.h \
           km_coffee/kmeans.h \
           km_coffee/Stack.h \
           km_coffee/Vector.h \
           platform_lib/platform_lib.h \
           t_coffee_lib/t_coffee.h \
           util_lib/aln_compare.h \
           util_lib/aln_convertion_util.h \
           util_lib/hsearch.h \
           util_lib/random.h \
           util_lib/reformat.h \
           util_lib/reformat_struc.h \
           util_lib/util.h \
           util_lib/util_aln_analyze.h \
           util_lib/util_analyse_constraints_list.h \
           util_lib/util_constraints_list.h \
           util_lib/util_declare.h \
           util_lib/util_domain_constraints_list.h \
           util_lib/util_dps.h \
           util_lib/util_job_handling.h \
           util_lib/util_make_tree.h
SOURCES += dev_lib/dev1.c \
           dp_lib/CUSTOM_evaluate_for_struc.c \
           dp_lib/evaluate.c \
           dp_lib/evaluate_dirichlet.c \
           dp_lib/evaluate_for_domain.c \
           dp_lib/evaluate_for_struc.c \
           dp_lib/fsa_dp.c \
           dp_lib/pavie_dp.c \
           dp_lib/showpair.c \
           dp_lib/util_domain_dp.c \
           dp_lib/util_domain_dp_drivers.c \
           dp_lib/util_dp_cdna_fasta_nw.c \
           dp_lib/util_dp_clean_maln.c \
           dp_lib/util_dp_drivers.c \
           dp_lib/util_dp_est.c \
           dp_lib/util_dp_fasta_nw.c \
           dp_lib/util_dp_fasta_sw.c \
           dp_lib/util_dp_generic_fasta_nw.c \
           dp_lib/util_dp_gotoh_nw.c \
           dp_lib/util_dp_gotoh_sw.c \
           dp_lib/util_dp_mm_nw.c \
           dp_lib/util_dp_sim.c \
           dp_lib/util_dp_ssec_pwaln.c \
           dp_lib/util_dp_suboptimal_nw.c \
           dp_lib/util_graph_maln.c \
           fastal/diagonal.c \
           fastal/fastal.c \
           fastal/fastal_opt_parsing.c \
           fastal/iteration.c \
           fastal/parttree.c \
           fastal/scoring.c \
           fastal/tree.c \
           io_lib/io_func.c \
           io_lib/pb_util_read_seq_util.c \
           io_lib/pb_util_read_sequence.c \
           io_lib/tree_util.c \
           km_coffee/classes.c \
           km_coffee/km_coffee.c \
           km_coffee/km_util.c \
           km_coffee/kmeans.c \
           km_coffee/Stack.c \
           km_coffee/Vector.c \
           platform_lib/platform_lib.c \
           t_coffee_lib/t_coffee.c \
           util_lib/aln_compare.c \
           util_lib/aln_convertion_util.c \
           util_lib/hsearch.c \
           util_lib/random.c \
           util_lib/reformat.c \
           util_lib/reformat_struc.c \
           util_lib/util.c \
           util_lib/util_aln_analyze.c \
           util_lib/util_analyse_constraints_list.c \
           util_lib/util_constraints_list.c \
           util_lib/util_declare.c \
           util_lib/util_domain_constraints_list.c \
           util_lib/util_dps.c \
           util_lib/util_job_handling.c \
           util_lib/util_make_tree.c