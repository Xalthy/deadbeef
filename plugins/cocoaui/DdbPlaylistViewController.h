//
//  DdbPlaylistViewController.h
//  deadbeef
//
//  Created by waker on 03/10/14.
//  Copyright (c) 2014 Alexey Yakovenko. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define PLT_MAX_COLUMNS 100

typedef struct {
    char *title;
    int _id; // predefined col type
    char *format;
    int size;
    char *bytecode;
    int bytecode_len;
} plt_col_info_t;

@interface DdbPlaylistViewController : NSViewController {
    plt_col_info_t columns[PLT_MAX_COLUMNS];
    int ncolumns;
    NSImage *playTpl;
    NSImage *pauseTpl;
    NSImage *bufTpl;
    NSDictionary *_colTextAttrsDictionary;
    NSDictionary *_cellTextAttrsDictionary;
    NSDictionary *_cellSelectedTextAttrsDictionary;
    NSDictionary *_groupTextAttrsDictionary;
}

// playlist columns
@property (unsafe_unretained) IBOutlet NSPanel *addColumnPanel;
- (IBAction)addColumnClose:(id)sender;

@end