#ifndef LSD_UTILS_H

#define LSD_UTILS_H


#define LSDB_LINK_FRESH_TIME 10000000
void lsdb_to_lsa_info(const struct cr_lsdb_link_state* lsdb, struct link_state_adv* lsa);
void lsa_to_lsdb_info(const struct link_state_adv* lsa, struct cr_lsdb_link_state* lsdb);
int is_out_of_date(const struct cr_lsdb_link_state* old);
int link_state_compare(const struct cr_lsdb_link_state* state1, const struct cr_lsdb_link_state* state2);

#endif /* end of include guard: LSD_UTILS_H */
