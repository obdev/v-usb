#!/usr/bin/awk -f
# Name: compare-sizes.awk
# Project: v-usb
# Author: Christian Starkjohann
# Creation Date: 2008-04-29
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

BEGIN{
	opt = 0;
	if(ARGC != 3){
		printf("usage: compare-sizes.awk file1 file2\n");
		printf("  computes size differences between two size lists\n");
		exit 1;
	}
	file1 = ARGV[1];
	file2 = ARGV[2];
}

{
	if(($2 + 0) != 0){
		if(!hadOption[$1]){
			hadOption[$1] = 1;
			options[opt++] = $1;
		}
		flash[FILENAME, $1] = $2;
		ram[FILENAME, $1] = $3;
	}
}

END{
	if(opt > 0){
		printf ("%39s %6s %5s\n", "Variation", "+Flash", "+RAM");
	}
	for(i = 0; i < opt; i++){
		option = options[i];
        if(!flash[file2, option] || !flash[file1, option]){
            printf("%39s %6s %5s\n", option, "n/a", "n/a");
        }else{
            printf("%39s %+6d %+5d\n", option, flash[file2, option] - flash[file1, option], ram[file2, option] - ram[file1, option]);
        }
	}
}
