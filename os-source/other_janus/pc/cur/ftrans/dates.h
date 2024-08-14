void date_normal_to_msdos(int year, int month, int day, unsigned *msdos);
void time_normal_to_msdos(int hour, int minute, int second, unsigned *msdos);
void date_normal_to_amiga(int year, int month, int day, struct DateStamp *ds);
void time_normal_to_amiga(int hour, int minute, int second, struct DateStamp *ds);
void date_msdos_to_normal(unsigned msdos, int *year, int *month, int *day);
void time_msdos_to_normal(unsigned msdos, int *hour, int *minute, int *second);
void date_amiga_to_normal(struct DateStamp *ds, int *year, int *month, int *day);
void time_amiga_to_normal(struct DateStamp *ds, int *hour, int *minute, int *second);
