#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INVALID -1 /* setted as the symbol of the non-leaf nodes */
#define HOW_MANY_POSSIBLE_SYMBOLS 256 /* how many possible symbols */

enum { ENCODE, DECODE }; /* program options */

/* Describes each tree node */
struct node {
    int isZero;
    int isRoot;
    int isLeaf;
    
    struct node *parent;
    struct node *leftChild;
    struct node *rightChild;
    
    unsigned char symbol;
    int value;
    int order;
};
typedef struct node Node;

/* Describes each symbol, containing a pointer to its node in the tree */
struct symbol {
    char symbol;
    Node *tree;
};
typedef struct symbol Symbol;

/* Creates the initial tree just containing the root node.
 * returns: a pointer to the root.
 */
Node* createTree() {
    Node *tree = malloc(sizeof(Node));
    tree->isZero = 1;
    tree->isRoot = 1;
    tree->isLeaf = 1;
    
    tree->parent = NULL;
    tree->leftChild = NULL;
    tree->rightChild = NULL;
    
    tree->symbol = INVALID;
    tree->value = 0;
    tree->order = HOW_MANY_POSSIBLE_SYMBOLS * 2; /* generally 512 (considering char = 8 bits) */
    
    return tree;
}

/* Obtains the tree node based on its symbol.
 * symbol: the desired symbol.
 * symbols: the array of all symbols.
 * returns: a pointer to the tree node corresponding to the passed symbol. If the node does not exist, a NULL pointer is returned.
 */
Node* getTreeFromSymbol(unsigned char symbol, Symbol **symbols) {
    Symbol *symbolPtr = symbols[(unsigned int)symbol];

    if (!symbolPtr) {
        return NULL;
    }
    
    return symbolPtr->tree;
}

/* Reverse an array of integers
 * code: the array that will be reserved;
 * codeSize: how many elements are contained in the array.
 */
void reverseCode(int *code, int codeSize) {
    if (code == NULL) {
        printf("reverseCode: tried to reverse a NULL code.\n");
        return;
    }
    
    int *start = code;
    int *end = code+(codeSize-1);
    
    while (start < end) {
        int temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

/* Returns an array of integers (i.e. 0 and 1s) containing the code corresponding to the given node.
 * node: the tree node.
 * n: the number of digits in the array that will be returned.
 * returns an array of integers containing the code.
 */
int* codeOfNode(Node *node, int *n) {
    Node *current = node;
    int *code = malloc(sizeof(int) * HOW_MANY_POSSIBLE_SYMBOLS * 2); /* worst case */
    
    int i = 0;
    while (!current->isRoot) {
        Node *parent = current->parent;
        code[i] = (parent->leftChild == current) ? 0 : 1;
        current = current->parent;
        i++;
    }
    reverseCode(code, i);
    
    *n = i;
    return code;
}

/* Creates a new child.
 * parent: the parent node that will be created.
 * isZero: 1 if the node is a ZERO node, or 0 otherwise.
 * isRoot: 1 if the created node is a root node, or 0 otherwise.
 * symbol: the symbol attributed to the node.
 * value: the value attributed to the node.
 * order: the order attributed to the node.
 * returns: the created node.
 */
Node* addChild(Node *parent, int isZero, int isRoot, unsigned char symbol, int value, int order) {
    Node *node = malloc(sizeof(Node));
    node->isZero = isZero;
    node->isRoot = isRoot;
    node->isLeaf = 1;
    node->parent = parent;
    node->leftChild = NULL;
    node->rightChild = NULL;
    node->symbol = symbol;
    node->value = value;
    node->order = order;
    return node;
}

/* Adds a new symbol in the tree and in the array of symbols
 * symbol: the symbol that will be inserted
 * zeroNode: reference to where the node ZERO points to.
 * symbols: the array of symbols
 * returns: the leaf node's (that contains the added symbol) parent.
 */
Node* addSymbol(unsigned char symbol, Node **zeroNode, Symbol **symbols) {
    Node *leftNode = addChild(*zeroNode, 1, 0, INVALID, 0, (*zeroNode)->order - 2);
    Node *rightNode = addChild(*zeroNode, 0, 0, symbol, 1, (*zeroNode)->order - 1);
    
    Node *previousZeroNode = *zeroNode;
    (*zeroNode)->isZero = 0;
    (*zeroNode)->isLeaf = 0;
    (*zeroNode)->leftChild = leftNode;
    (*zeroNode)->rightChild = rightNode;
    
    unsigned int symbolIndex = (unsigned int)symbol;
    symbols[symbolIndex] = malloc(sizeof(Symbol));
    symbols[symbolIndex]->symbol = symbol;
    symbols[symbolIndex]->tree = rightNode;
    
    *zeroNode = leftNode;
    
    return previousZeroNode;
}

/* Searches for the node that has the same value as the input node (with a maximum order value).
 * currMax: the input node.
 * root: the tree's root.
 * returns: the node that has the same value as the input node (with a maximum order value). if it doesn't exist, NULL is returned.
 */
Node* findReplaceNode(Node *currMax, Node *root) {
    Node *result = currMax;
    
    if (root->value > result->value && !root->isLeaf) {
        Node *greatestLeft = findReplaceNode(result, root->leftChild);
        if (greatestLeft) result = greatestLeft;
        
        Node *greatestRight = findReplaceNode(result, root->rightChild);
        if (greatestRight) result = greatestRight;
    } else if (root->value == result->value && root->order > result->order) {
        result = root;
    }
    
    return (result != currMax) ? result : NULL;
}

/* Swap nodes.
 * Note: the orders are not swapped, since they are independent from the node's content.
 */
void swapNodes(Node *n1, Node *n2) {
    /* mudando as ordens porque elas nao se trocam */
    int tempOrder = n1->order;
    n1->order = n2->order;
    n2->order = tempOrder;
    
    if (n1->parent->leftChild == n1) {
        n1->parent->leftChild = n2;
    } else {
        n1->parent->rightChild = n2;
    }
    
    if (n2->parent->leftChild == n2) {
        n2->parent->leftChild = n1;
    } else {
        n2->parent->rightChild = n1;
    }
    
    Node *temp = n1->parent;
    n1->parent = n2->parent;
    n2->parent = temp;
}

/* Updates the tree based on an initial node.
 * currNode: the starting node that will be updated.
 * root: the tree's root.
 */
void updateTree(Node *currNode, Node *root) {
    while (!currNode->isRoot) {
        Node *replaceNode = findReplaceNode(currNode, root);
        
        /* se replaceNode for o pai de currentNode, eles nao podem ser trocados */
        if (replaceNode && currNode->parent != replaceNode) {
            swapNodes(currNode, replaceNode);
        }
        
        (currNode->value)++;
        currNode = currNode->parent;
    }
    
    (currNode->value)++;
}

/* Adds a code (of 0 and 1s) to the output buffer. When the buffer gets full, it will be written in the file.
 * code: the code that will be added into the output buffer. WHen the buffer gets full, it will be written in the file.
 * codeSize: how many bits are contained in code.
 * fp: the output file.
 * buffer: the input buffer.
 * bufferSize: how many bits in buffer are waiting to be written.
 * returns: the new buffer. 
 */
unsigned char addCodeToBuffer(int *code, int codeSize, FILE *fp, unsigned char buffer, int *bufferSize) {
    unsigned char currBuffer = buffer;
    
    int i;
    for (i = 0; i < codeSize; i++) {
        unsigned char bit = ((unsigned char) code[i]) << (*bufferSize - 1);
        currBuffer = currBuffer | bit;
        
        (*bufferSize)--;
        
        if (*bufferSize == 0) {
            fwrite(&currBuffer, sizeof(unsigned char), 1, fp);
            currBuffer = 0;
            *bufferSize = 8;
        }
    }
    
    return currBuffer;
}

/* Adds a byte into the output buffer. If the buffer is not previously empty, a byte will be written in the output file.
 * byte: the byte that will be written in the buffer.
 * fp: The output file.
 * buffer: the input buffer.
 * bufferSize: how many bits in the buffer are waiting to be written.
 * returns: the new buffer.
 */
unsigned char addByteToBuffer(char byte, FILE *fp, unsigned char buffer, int *bufferSize) {
    unsigned char currBuffer = buffer;
    
    int howManyBitsWillBeWritten = *bufferSize;
    int shiftSize = 8 - howManyBitsWillBeWritten;
    unsigned char tempByte = ((unsigned char) byte) >> shiftSize;
    currBuffer = currBuffer | tempByte;
    
    fwrite(&currBuffer, sizeof(unsigned char), 1, fp);
    
    currBuffer = byte << howManyBitsWillBeWritten;
    
    return (*bufferSize == 8) ? 0 : currBuffer;
}

/* Writes the buffer in the output file.
 * Also writes the last byte containing the amount of bits that the decoder will read in the next to last byte.
 *
 * fp: the output file.
 * buffer: the input buffer.
 * bufferSize: how many bits should be written in the buffer.
 */
void writeRemainingBits(FILE *fp, unsigned char buffer, int bufferSize) {
    if (bufferSize < 8) {
        fwrite(&buffer, sizeof(unsigned char), 1, fp);
    }
    
    /* Writes in the last byte how many bits should be read by the decoded in the next to last byte. */
    buffer = (bufferSize == 8) ? 8 : 8 - bufferSize;
    fwrite(&buffer, sizeof(unsigned char), 1, fp);
}

/* Encodes the file.
 * fp_in: the input file (the one that will be encoded).
 * fp_out: the output file.
 */
void encode(FILE *fp_in, FILE *fp_out) {
    Node *root = createTree();
    Node *zeroNode = root;
    
    unsigned char buffer = 0;
    int bufferSize = 8;
    
    Symbol **symbols = calloc(HOW_MANY_POSSIBLE_SYMBOLS, sizeof(Symbol*)); /* initializing with 0s */
    
    unsigned char currByte;
    while (fread(&currByte, sizeof(unsigned char), 1, fp_in) > 0) {
        Node *symbolTree = getTreeFromSymbol(currByte, symbols);
        
        if (symbolTree) {
            int codeSize;
            int *symbolCode = codeOfNode(symbolTree, &codeSize);
            buffer = addCodeToBuffer(symbolCode, codeSize, fp_out, buffer, &bufferSize);
            
            updateTree(symbolTree, root);
            free(symbolCode);
        } else {
            int codeSize;
            
            int *zeroCode = codeOfNode(zeroNode, &codeSize);
            buffer = addCodeToBuffer(zeroCode, codeSize, fp_out, buffer, &bufferSize);
            buffer = addByteToBuffer(currByte, fp_out, buffer, &bufferSize);
            
            Node *newNode = addSymbol(currByte, &zeroNode, symbols);
            updateTree(newNode, root);
            free(zeroCode);
        }
    }
    
    writeRemainingBits(fp_out, buffer, bufferSize);
}

/* Read the next bit to be read in the biffer and returns it.
 * fp: input file (that will be read in case the buffer limit is reached).
 * buffer: the input buffer, that can be updated in case a read operation if performed in the input file.
 * bufferSize: how many bits there are left to be read in the buffer.
 * fileSize: the filesize (until its next to last byte).
 * readHowManyBitsAtTheEnd: how many bits should be read in the next to last byte.
 * returns: the bit (0 or 1).
 */
int readBit(FILE *fp, unsigned char *buffer, int *bufferSize, long int fileSize, int readHowManyBitsAtTheEnd) {
    if (*bufferSize == 0) {
        *bufferSize = 8;
        fread(buffer, sizeof(unsigned char), 1, fp);
    }
    
    if (readHowManyBitsAtTheEnd != 8) {
        if (ftell(fp) == fileSize && readHowManyBitsAtTheEnd <= (8 - *bufferSize)) return -1;
    }
    
    if (ftell(fp) > fileSize || feof(fp)) return -1;
    
    (*bufferSize)--;
    
    return (*buffer >> *bufferSize) & 1;
}

/* Reads the next 8 bits to be read and return it as a char.
 * fp: input file (that will be read in case the buffer limit is reached).
 * buffer: the input buffer, that can be updated in case a read operation if performed in the input file.
 * bufferSize: how many bits there are left to be read in the buffer.
 * fileSize: the filesize (until its next to last byte).
 * readHowManyBitsAtTheEnd: how many bits should be read in the next to last byte.
 * returns: the read byte.
 */
char readByte(FILE *fp, unsigned char *buffer, int *bufferSize, long int fileSize, int readHowManyBitsAtTheEnd) {
    char result = 0;
    
    int i, bit;
    for (i = 0; i < 8; i++) {
        bit = readBit(fp, buffer, bufferSize, fileSize, readHowManyBitsAtTheEnd);
        bit = bit << (7-i);
        result |= bit;
    }
    
    return result;
}

/* Decodes the file
 * fp_in: input file (the one that will be decoded)
 * fp_out: the output file
 */
void decode(FILE *fp_in, FILE *fp_out) {
    Node *root = createTree();
    Node *zeroNode = root;
    
    unsigned char buffer = 0;
    int bufferSize = 0;
    
    Symbol **symbols = calloc(HOW_MANY_POSSIBLE_SYMBOLS, sizeof(Symbol*)); /* initializing with 0s */
    
    /* Obtains the file size (excluding the last byte). */
    fseek(fp_in, -1, SEEK_END);
    long int fileSize = ftell(fp_in);
    
    /* Reads the last byte, which contains the number of bits that should be read in the previous byte */
    unsigned char readHowManyBitsAtTheEnd;
    fread(&readHowManyBitsAtTheEnd, sizeof(unsigned char), 1, fp_in);
    rewind(fp_in);
    
    while (!feof(fp_in)) {
        Node *currNode = root;
        
        int endOfFile = 0;
        while (!currNode->isLeaf && !endOfFile) {
            int bit = readBit(fp_in, &buffer, &bufferSize, fileSize, readHowManyBitsAtTheEnd);
            if (bit == 0) {
                currNode = currNode->leftChild;
            } else if (bit == 1) {
                currNode = currNode->rightChild;
            } else {
                endOfFile = 1;
            }
        }
        
        if (endOfFile) break;
        
        unsigned char c;
        if (currNode->isZero) {
            c = readByte(fp_in, &buffer, &bufferSize, fileSize, readHowManyBitsAtTheEnd);
            currNode = addSymbol(c, &zeroNode, symbols);
        } else {
            c = currNode->symbol;
        }
        
        fwrite(&c, sizeof(unsigned char), 1, fp_out);
        updateTree(currNode, root);
    }
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
        printf("Usage:\n");
		printf("\t./fgk input_file output_file -c (to encode)\n");
		printf("\t./fgk input_file output_file -d (to decode)\n");
		exit(1);
	}
	
    FILE *fp_in;
    FILE *fp_out;
    int option;
    
    /* input file */
    fp_in = fopen(argv[1], "rb");
    while (fp_in == NULL) {
        char in_file[100];
        printf("The file %s could not be opened. Try again: ", argv[1]);
        scanf("%s", in_file);
        fp_in = fopen(in_file, "rb");
    }

    /* output file */
    fp_out = fopen(argv[2], "wb");

    /* option: encode or decode */
    if (strcmp(argv[3], "-c") == 0 || strcmp(argv[3], "-C") == 0) {
        option = ENCODE;
    } else if (strcmp(argv[3], "-d") == 0 || strcmp(argv[3], "-D") == 0) {
        option = DECODE;
    } else {
        char opt;
        do {
            printf("Invalid option, type 'c' to encode or 'd' to decode: ");
            scanf("%c", &opt);
            getchar();
        } while (opt != 'c' && opt != 'C' && opt != 'd' && opt != 'D');
        option = (opt == 'c' || opt == 'C') ? ENCODE : DECODE;
    }
    
    if (option == ENCODE) {
        encode(fp_in, fp_out);
        printf("The file was encoded successfully.\n");
    } else {
        decode(fp_in, fp_out);
        printf("The file was decoded successfully\n");
    }
    
    fclose(fp_in);
    fclose(fp_out);
    
    return 0;
}
