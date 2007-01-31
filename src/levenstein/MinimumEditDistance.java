import java.util.*;
import java.io.*;

/**
 * This class implements Levenstein's Minimum Edit Distance algorithm.
 * It includes a main method for processing a text file of words and
 * producing a list of possible spelling corrections.
 */
public class MinimumEditDistance {
    /**
    * Find the minimum of three values.
    */
    public static int minimum(int a, int b, int c) {
        int mi = a;
        if(b < mi) {
            mi = b;
        }
        if (c < mi) {
            mi = c;
        }
        return mi;
    }

    /**
     * Computes the Minimum Edit Distance between two strings.
     * Uses dynamic programming.
     */
    public static int minimumDistance(String s, String t) {
        int n = s.length();
        int m = t.length();
        int d[][];  // Matrix filled in by the algorithm; entry i,j represents the minimum
                    // edit distance between the i-prefix of s and the j-prefix of t.
        int i;      // iterates through s
        int j;      // iterates through t

        if(n == 0) {
            return m;
        }
        if(m == 0) {
            return n;
        }
        d = new int[n+1][m+1];

        // Initialize matrix with start costs
        for(i = 0; i <= n; i++) {
            d[i][0] = i;
        }
        for(j = 0; j <= m; j++) {
            d[0][j] = j;
        }

        // Fill in the matrix
        for(i = 1; i <= n; i++) {
            char s_i = s.charAt(i - 1);
            for (j = 1; j <= m; j++) {
                char t_j = t.charAt(j - 1);
                // The essence of the algorithm:
                d[i][j] = minimum(  d[i-1][j]+1,
                                    d[i][j-1]+1,
                                    d[i-1][j-1] + (s_i == t_j ? 0 : 1)  );
            }
        }
        return d[n][m];
    }

    /**
     * For each word in the misspeltWords list, a line is written to out containing the misspelt
     * word, and each possible correct word and its occurence. Possible correct words are taken to
     * be words starting with the same letter and having a minimum edit distance less than beta.
     *
     * The correctWords files contains lists of words starting with the same letter.
     */
    public static void findCorrectSpellings(String filename, int beta, PrintWriter out, boolean verbose) throws IOException
    {
        if(verbose)
            System.out.print("Finding correct spellings");
        BufferedReader br = new BufferedReader(new FileReader(filename+"_misspelt.txt"));
        String line = br.readLine();
        while(line != null) {
            if(verbose)
                System.out.print(".");
            WordOccurence wo = new WordOccurence(line);
            String firstChar = wo.word.substring(0,1);
            BufferedReader candList = new BufferedReader(new FileReader(filename+"_"+firstChar+".txt"));
            String output = wo.word+"=";
            boolean found = false;
            String cand = candList.readLine();
            while(cand != null) {
                WordOccurence candWord = new WordOccurence(cand);
                int dist = minimumDistance(wo.word, candWord.word);
                if(dist < beta) {
                    output += candWord.word+"("+dist+","+candWord.occurence+");";
                    found = true;
                }
                cand = candList.readLine();
            }
            if(found)
                out.println(output);
            candList.close();
            line = br.readLine();
        }
        br.close();
        if(verbose)
            System.out.println("done");
    }
    
    /**
     * Processes a vocabulary list and looks for words occuring fewer than alpha times in the corpus.
     * These words are compared with other words starting with the same letter. If the minimum edit
     * distance is smaller than beta, the first word is taken as a possible misspelling of the second.
     * Results are outputted to a file where each line is formatted like this:
     * <misspelt word>=<<possible correct spelling>(<minimum edit distance>:<occurence>);>*
     *
     * The occurence is the number of times a word occurs in the corpus, as given in the input file.
     * 
     * Example input:
     * test 129
     * trapp 2000
     * trap 9
     * mann 239
     * kake 3
     *
     * Example output:
     * trap=trapp(1,2000);test(3,129);
     *
     * The method takes the following parameter list from the command line:
     * <input file> <alpha> <beta> [<output file>]
     *
     * If output file is not specified, output is printed to system.out.
     */
    public static void main(String args[]) {
        if(args.length < 3) {
            System.out.println("Usage: java MinimumEditDistance <input file> <alpha> <beta> [<output file>]");
            System.exit(0);
        }
        try {
            BufferedReader br = new BufferedReader(new FileReader(args[0]));
            long alpha = Long.parseLong(args[1]);
            int beta = Integer.parseInt(args[2]);
            PrintWriter pw;
            boolean verbose;
            if(args.length > 3) {
                pw = new PrintWriter(new FileWriter(args[3]));
                verbose = true;
            }
            else {
                pw = new PrintWriter(System.out);
                verbose = false;
            }
            
            Hashtable correctWords = new Hashtable(); // Hashtable keeping files with words that are not misspelt, one for each letter
            PrintWriter misspeltWords = new PrintWriter(new FileWriter(args[0]+"_misspelt.txt"));

            // Preprocessing of input file: Input file is split up into files where all words start
            // with the same letter, plus one file with possibly misspelt words.
            if(verbose)
                System.out.print("Reading input file");
            int count = 0;
            String line = br.readLine();
            while(line != null) {
                count++;
                if(verbose && count % 1000 == 0)
                    System.out.print(".");
                WordOccurence wo = new WordOccurence(line);
                if(wo.occurence < alpha) {
                    misspeltWords.println(wo);
                }
                else {
                    String firstChar = wo.word.substring(0,1);
                    PrintWriter file = (PrintWriter)correctWords.get(firstChar);
                    if(file == null) {
                        // Create a new file
                        file = new PrintWriter(new FileWriter(args[0]+"_"+firstChar+".txt"));
                        correctWords.put(firstChar, file);
                    }
                    file.println(wo);
                }
                line = br.readLine();
            }
            br.close();
            misspeltWords.close();
            for(Enumeration e = correctWords.elements(); e.hasMoreElements(); ) {
                PrintWriter file = (PrintWriter)e.nextElement();
                file.close();
            }
            if(verbose)
                System.out.println("done");

            // Create the actual output file
            findCorrectSpellings(args[0], beta, pw, verbose);

            // Close output if it is not system.out
            if(verbose)
                pw.close();
        }
        catch(IOException ioe) {
            ioe.printStackTrace();
        }
    }
}

/**
 * "Struct" containing a word and its occurence in a corpus.
 */
class WordOccurence {
    public String word;
    public long occurence;

    public WordOccurence(String word, long occurence) {
        this.word = word;
        this.occurence = occurence;
    }

    public WordOccurence(String line) {
        int i = line.indexOf(" ");
        if(i > -1) {
            word = line.substring(0,i);
            occurence = Long.parseLong(line.substring(i+1));
        }
        else {
            word = line;
            occurence = 0;
        }
    }

    public String toString() {
        return word+" "+occurence;
    }
}