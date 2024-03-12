/**********************************************************
 * File: HuffmanEncoding.cpp
 *
 * Implementation of the functions from HuffmanEncoding.h.
 * Most (if not all) of the code that you write for this
 * assignment will go into this file.
 */

#include "HuffmanEncoding.h"
/* Function: getFrequencyTable
 * Usage: Map<ext_char, int> freq = getFrequencyTable(file);
 * --------------------------------------------------------
 * Given an input stream containing text, calculates the
 * frequencies of each character within that text and stores
 * the result as a Map from ext_chars to the number of times
 * that the character appears.
 *
 * This function will also set the frequency of the PSEUDO_EOF
 * character to be 1, which ensures that any future encoding
 * tree built from these frequencies will have an encoding for
 * the PSEUDO_EOF character.
 */
Map<ext_char, int> getFrequencyTable(istream& file) {
	// TODO: Implement this!
	Map<ext_char, int> table;
	char c;
	while(file.get(c)) {
		if(!table.containsKey(c)) {
			table.put(c, 1);
		}
		else {
			table[c]++;
		}
	}
	table[PSEUDO_EOF]++;
	return table;	
}

/* Function: buildEncodingTree
 * Usage: Node* tree = buildEncodingTree(frequency);
 * --------------------------------------------------------
 * Given a map from extended characters to frequencies,
 * constructs a Huffman encoding tree from those frequencies
 * and returns a pointer to the root.
 *
 * This function can assume that there is always at least one
 * entry in the map, since the PSEUDO_EOF character will always
 * be present.
 */
Node* buildEncodingTree(Map<ext_char, int>& frequencies) {
    PriorityQueue<Node*> queue;
	foreach(ext_char c in frequencies) {
		int frequency = frequencies[c];
		Node* leaf = new Node;
		leaf -> character = c;
		leaf -> zero = NULL;
		leaf -> one = NULL;
		leaf -> weight = frequency;
		queue.enqueue(leaf, frequency);
	}
	while(true) {
		Node* zero = queue.dequeue();
		Node* one = queue.dequeue();
		int frequency = one -> weight + zero -> weight;
		Node* parent = new Node;
		parent -> zero = zero;
		parent -> one = one;
		parent -> character = NOT_A_CHAR;
		parent -> weight = frequency;
		queue.enqueue(parent, frequency);
		if(queue.size() == 1) {
			return queue.dequeue();
		}
	}
}

/* Function: freeTree
 * Usage: freeTree(encodingTree);
 * --------------------------------------------------------
 * Deallocates all memory allocated for a given encoding
 * tree.
 */
void freeTree(Node* root) {
	// TODO: Implement this!
	if(root == NULL) {
		return;
	}
	freeTree(root -> zero);
	freeTree(root -> one);
	delete root;
}

/* Function: encodeFile
 * Usage: encodeFile(source, encodingTree, output);
 * --------------------------------------------------------
 * Encodes the given file using the encoding specified by the
 * given encoding tree, then writes the result one bit at a
 * time to the specified output file.
 *
 * This function can assume the following:
 *
 *   - The encoding tree was constructed from the given file,
 *     so every character appears somewhere in the encoding
 *     tree.
 *
 *   - The output file already has the encoding table written
 *     to it, and the file cursor is at the end of the file.
 *     This means that you should just start writing the bits
 *     without seeking the file anywhere.
 */ 
void encodeFile(istream& infile, Node* encodingTree, obstream& outfile) {
	// TODO: Implement this!
	Map<ext_char, string> encodingMap;
	Set<Node*> visited;
	string code = "";
	generateMap(encodingTree, code, encodingMap, visited);
	char c;
	while(infile.get(c)) {
		code = encodingMap[c];
		for(int i = 0; i < code.size(); i++) {
			outfile.writeBit(code[i] - '0');
		}
	}
	code = encodingMap[PSEUDO_EOF];
	for(int i = 0; i < code.size(); i++) {
		outfile.writeBit(code[i] - '0');
	}
}

/* Function: decodeFile
 * Usage: decodeFile(encodedFile, encodingTree, resultFile);
 * --------------------------------------------------------
 * Decodes a file that has previously been encoded using the
 * encodeFile function.  You can assume the following:
 *
 *   - The encoding table has already been read from the input
 *     file, and the encoding tree parameter was constructed from
 *     this encoding table.
 *
 *   - The output file is open and ready for writing.
 */
void decodeFile(ibstream& infile, Node* encodingTree, ostream& file) {
	// TODO: Implement this!
	Node* curr = encodingTree;
	int bit = 0;
	while(true) {
		bit = infile.readBit();
		if(bit == 0) {
			curr = curr -> zero;
		}
		else {
			curr = curr -> one;
		}
		if(curr -> character == PSEUDO_EOF) {
			break;
		}
		else if(curr -> zero == NULL && curr -> one == NULL) {
			file.put(curr -> character);
			curr = encodingTree;
		}
	}
}

/* Function: writeFileHeader
 * Usage: writeFileHeader(output, frequencies);
 * --------------------------------------------------------
 * Writes a table to the front of the specified output file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to decompress input files once they've been
 * compressed.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * readFileHeader function defined below this one so that it
 * can properly read the data back.
 */
void writeFileHeader(obstream& outfile, Map<ext_char, int>& frequencies) {
	/* The format we will use is the following:
	 *
	 * First number: Total number of characters whose frequency is being
	 *               encoded.
	 * An appropriate number of pairs of the form [char][frequency][space],
	 * encoding the number of occurrences.
	 *
	 * No information about PSEUDO_EOF is written, since the frequency is
	 * always 1.
	 */
	 
	/* Verify that we have PSEUDO_EOF somewhere in this mapping. */
	if (!frequencies.containsKey(PSEUDO_EOF)) {
		error("No PSEUDO_EOF defined.");
	}
	
	/* Write how many encodings we're going to have.  Note the space after
	 * this number to ensure that we can read it back correctly.
	 */
	outfile << frequencies.size() - 1 << ' ';
	
	/* Now, write the letter/frequency pairs. */
	foreach (ext_char ch in frequencies) {
		/* Skip PSEUDO_EOF if we see it. */
		if (ch == PSEUDO_EOF) continue;
		
		/* Write out the letter and its frequency. */
		outfile << char(ch) << frequencies[ch] << ' ';
	}
}

/* Function: readFileHeader
 * Usage: Map<ext_char, int> freq = writeFileHeader(input);
 * --------------------------------------------------------
 * Reads a table to the front of the specified input file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to reconstruct the encoding tree for that file.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * writeFileHeader function defined before this one so that it
 * can properly write the data.
 */
Map<ext_char, int> readFileHeader(ibstream& infile) {
	/* This function inverts the mapping we wrote out in the
	 * writeFileHeader function before.  If you make any
	 * changes to that function, be sure to change this one
	 * too!
	 */
	Map<ext_char, int> result;
	
	/* Read how many values we're going to read in. */
	int numValues;
	infile >> numValues;
	
	/* Skip trailing whitespace. */
	infile.get();
	
	/* Read those values in. */
	for (int i = 0; i < numValues; i++) {
		/* Get the character we're going to read. */
		ext_char ch = infile.get();
		
		/* Get the frequency. */
		int frequency;
		infile >> frequency;
		
		/* Skip the space character. */
		infile.get();
		
		/* Add this to the encoding table. */
		result[ch] = frequency;
	}
	
	/* Add in 1 for PSEUDO_EOF. */
	result[PSEUDO_EOF] = 1;
	return result;
}

/* Function: compress
 * Usage: compress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman compressor.  Compresses
 * the file whose contents are specified by the input
 * ibstream, then writes the result to outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void compress(ibstream& infile, obstream& outfile) {
	// TODO: Implement this!
	Map<ext_char, int> freqTable = getFrequencyTable(infile);
	Node* encodingTree = buildEncodingTree(freqTable);
	writeFileHeader(outfile, freqTable);
	infile.rewind();
	encodeFile(infile, encodingTree, outfile);
	freeTree(encodingTree);
}

/* Function: decompress
 * Usage: decompress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman decompressor.
 * Decompresses the file whose contents are specified by the
 * input ibstream, then writes the decompressed version of
 * the file to the stream specified by outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void decompress(ibstream& infile, ostream& outfile) {
	// TODO: Implement this!
	Map<ext_char, int> freqTable = readFileHeader(infile);
	Node* encodingTree = buildEncodingTree(freqTable);
	decodeFile(infile, encodingTree, outfile);
	freeTree(encodingTree);;
}

void generateMap(Node* nodes, string& code, Map<ext_char, string>& encodingMap, Set<Node*> visited) {
	if(nodes == NULL) {
		return;
	}
	else if(nodes -> character != NOT_A_CHAR && !visited.contains(nodes)) {
		encodingMap[nodes -> character] = code;
		visited.add(nodes);
	}
	else {
		code += "0";
		generateMap(nodes -> zero, code, encodingMap, visited);
		code = code.substr(0, code.length() - 1);
		code += "1";
		generateMap(nodes -> one, code, encodingMap, visited);
		code = code.substr(0, code.length() - 1);
	}
}