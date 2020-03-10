/*
 * cc  geo_mmdb_lookup.c -o geo_mmdb_lookup -l maxminddb
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <maxminddb.h>
#include <stdlib.h>
#include <string.h>

char *asfile = "/usr/share/GeoIP/GeoLite2-ASN.mmdb";
char *cityfile = "/usr/share/GeoIP/GeoLite2-City.mmdb";


bool isValidIPv4Address(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

bool isValidIPv6Address(char *ipAddress) {
    struct sockaddr_in6 sa6;
    int result6 = inet_pton(AF_INET6, ipAddress, &(sa6.sin6_addr));
    return result6 != 0;
}

char * get_value(MMDB_lookup_result_s *result, const char **path) {

    MMDB_entry_data_s entry_data;
    int status  = MMDB_aget_value( &(*result).entry, &entry_data, path);
    char *value = NULL;

    if (MMDB_SUCCESS != status) {
        #ifdef DEBUG
        fprintf(
                stderr,
                "[WARN] get_value got an error looking up the entry data. Make sure you use the correct path - %s\n",
                MMDB_strerror(status));
        #endif
        return NULL;
    }
    
    if (entry_data.has_data) {
        switch(entry_data.type) {           
        case MMDB_DATA_TYPE_UTF8_STRING:
            value = strndup(entry_data.utf8_string, entry_data.data_size);
            break;
        case MMDB_DATA_TYPE_UINT16:
            value = malloc(entry_data.data_size);
            sprintf(value, "%u", entry_data.uint16);
            break;
        case MMDB_DATA_TYPE_UINT32:
            value = malloc(entry_data.data_size);
            sprintf(value, "%u", entry_data.uint32);
            break;
        case MMDB_DATA_TYPE_DOUBLE:
            asprintf(&value, "%f", entry_data.double_value);
            break;
        default:
            #ifdef DEBUG 
            fprintf(
                    stderr,
                    "[WARN] get_value: No handler for entry data type (%d) was found. \n",
                    entry_data.type);
            #endif 
            break;
        }
    }
    return value;
}

char * geo_lookup_as(const char *ip_address) 
{
    char *asnumber = NULL;
    char *asname   = NULL;
    char *as = malloc(256);
    int asname_max = 48;

    const char *asnumber_lookup[] = {"autonomous_system_number", NULL};
    const char *asname_lookup[]   = {"autonomous_system_organization", NULL};

    MMDB_s asmmdb;
    int asstatus = MMDB_open(asfile, MMDB_MODE_MMAP, &asmmdb);

    if (MMDB_SUCCESS != asstatus) {
           fprintf(stderr, "\n  Can't open %s - %s\n", asfile, MMDB_strerror(asstatus));

           if (MMDB_IO_ERROR == asstatus) {
              fprintf(stderr, "    IO error: %s\n", strerror(errno));
           } 
        exit(1);
    }

    int asgai_error, asmmdb_error;
    MMDB_lookup_result_s asresult =
        MMDB_lookup_string(&asmmdb, ip_address, &asgai_error, &asmmdb_error);

    if (0 != asgai_error) {
        fprintf(stderr, "\n  Error from AS getaddrinfo for %s - %s\n\n", ip_address, gai_strerror(asgai_error)); 
        exit(2);
    }

    if (MMDB_SUCCESS != asmmdb_error) {
        fprintf(stderr, "\n  Got an error from AS libmaxminddb: %s\n\n", MMDB_strerror(asmmdb_error)); 
        exit(3);
    }

    MMDB_entry_data_list_s *asentry_data_list = NULL;

    int asexit_code = 0;
    if (asresult.found_entry) {
        int status = MMDB_get_entry_data_list(&asresult.entry,
                                              &asentry_data_list);

        if (MMDB_SUCCESS != status) {
            fprintf( stderr, "Got an error looking up the AS entry data - %s\n", MMDB_strerror(status)); 
            asexit_code = 4;
            goto end;
        }

        if (NULL != asentry_data_list) {

	    asnumber = get_value(&asresult, asnumber_lookup);
	    asname = get_value(&asresult, asname_lookup);
        }

    } else {
         fprintf( stderr, "\n  No AS entry for this IP address (%s) was found\n\n", ip_address); 
       	 asexit_code = 5;
    }

    end:
        MMDB_free_entry_data_list(asentry_data_list);
        MMDB_close(&asmmdb);

	if(asexit_code > 0 || asnumber == NULL) {
		sprintf(as, "Unknown - Exit Code: %d", asexit_code); 
	} else {
		if (asname == NULL) {
            		asname = strdup("Unknown"); 
		}

		if (strlen(asname) > asname_max) {
                        asname[asname_max] = '\0';
                }

		sprintf(as, "AS%s: %s", asnumber, asname); 

		if (asname != NULL ) {
                        free(asname);
                }
                if (asnumber != NULL ) {
                        free(asnumber);
                }
	}
	return strdup(as); 
}


int main(int argc, char **argv)
{

    if (argc != 2) {
        printf("\n%s <ip address>\n\n", argv[0]);
        exit(-1);
    }

    char *ip_address = argv[1];

    if (!isValidIPv4Address(ip_address) && !isValidIPv6Address(ip_address)) {
        fprintf(stderr, "\n  Not a valid IP address.\n\n");
        exit(-1);
    }

    char *continentcode   = NULL;
    char *continent   = NULL;
    char *countrycode   = NULL;
    char *country = NULL;
    char *city    = NULL;
    char *state   = NULL;
    char *province   = NULL;
    char *latitude   = NULL;
    char *longitude   = NULL;
    char *as = NULL;
    char *data;

    const char *continentcode_lookup[]   = {"continent", "code", NULL};
    const char *continent_lookup[]   = {"continent", "names", "en", NULL};
    const char *countrycode_lookup[] = {"country", "iso_code", NULL};
    const char *country_lookup[] = {"country", "names", "en", NULL};
    const char *city_lookup[]    = {"city", "names", "en", NULL};
    const char *state_lookup[]   = {"subdivisions", "0", "iso_code", NULL};
    const char *province_lookup[]   = {"subdivisions", "0", "names", "en", NULL};
    const char *longitude_lookup[]   = {"location", "longitude", NULL};
    const char *latitude_lookup[]   = {"location", "latitude", NULL};

    MMDB_s mmdb;
    int status = MMDB_open(cityfile, MMDB_MODE_MMAP, &mmdb);

    if (MMDB_SUCCESS != status) {
          fprintf(stderr, "\n  Can't open %s - %s\n", cityfile, MMDB_strerror(status));

          if (MMDB_IO_ERROR == status) {
              fprintf(stderr, "    IO error: %s\n", strerror(errno));
          }
        exit(1);
    }

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result =
        MMDB_lookup_string(&mmdb, ip_address, &gai_error, &mmdb_error);

    if (0 != gai_error) {
        fprintf(stderr, "\n  Error from getaddrinfo for %s - %s\n\n", ip_address, gai_strerror(gai_error));
        exit(2);
    }

    if (MMDB_SUCCESS != mmdb_error) {
        fprintf(stderr, "\n  Got an error from libmaxminddb: %s\n\n", MMDB_strerror(mmdb_error));
        exit(3);
    }

    MMDB_entry_data_list_s *entry_data_list = NULL;

    int exit_code = 0;
    if (result.found_entry) {
        int status = MMDB_get_entry_data_list(&result.entry,
                                              &entry_data_list);

        if (MMDB_SUCCESS != status) {
            fprintf( stderr, "Got an error looking up the entry data - %s\n", MMDB_strerror(status));
            exit_code = 4;
            goto end;
        }

        if (NULL != entry_data_list) {

	    continentcode = get_value(&result, continentcode_lookup);
	    continent = get_value(&result, continent_lookup);
	    countrycode = get_value(&result, countrycode_lookup);
            country = get_value(&result, country_lookup);
            city    = get_value(&result, city_lookup);		
	    province = get_value(&result, province_lookup);
            state = get_value(&result, state_lookup);
	    latitude = get_value(&result, latitude_lookup);
	    longitude = get_value(&result, longitude_lookup);
   	    as = geo_lookup_as(ip_address);

	    /* if (countrycode != NULL && strcmp(countrycode,"US") == 0) { 
	    if (countrycode != NULL) {
            	state = get_value(&result, state_lookup);
            } else {
            	state = strdup("--");
            }
	    */

            if (state == NULL) {
            	state = strdup("--"); 
            }
            if (city == NULL) {
            	city = strdup("--"); 
            }
            if (province == NULL) {
            	 province = strdup("--"); 
            }

	    /* MMDB_dump_entry_data_list(stdout, entry_data_list, 2); */

   	    fprintf( stdout, "\n  Address:\t%s\n", ip_address); 
   	    fprintf( stdout, "  CountryCode:\t%s\n", countrycode); 
   	    fprintf( stdout, "  Country:\t%s: %s, %s: %s\n", continentcode, continent, countrycode, country); 
   	    fprintf( stdout, "  City:\t\t%s, %s, %s, %s, %s\n", state, city, province, latitude, longitude); 
   	    fprintf( stdout, "  AS Number:\t%s\n\n", as); 

	    if (latitude != NULL) {
                free(latitude);
            }
            if (longitude != NULL) {
                free(longitude);
            }
            if (continentcode != NULL) {
                free(continentcode);
            }
            if (country != NULL) {
                free(country);
            }
            if (countrycode != NULL) {
                free(countrycode);
            }
            if (continent != NULL) {
                free(continent);
            }
            if (state != NULL) {
                free(state);
            }
            if (city != NULL) {
                free(city);
            }
            if (province != NULL) {
                free(province);
            }
            if (as != NULL) {
                free(as);
            }
        }

    } else {
        fprintf( stderr, "\n  No entry for this IP address (%s) was found\n\n", ip_address);
        exit_code = 5;
    }

    end:
        MMDB_free_entry_data_list(entry_data_list);
        MMDB_close(&mmdb);
        exit(exit_code);
}
