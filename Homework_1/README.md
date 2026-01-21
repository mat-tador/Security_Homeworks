# Cracking a Substitution Cipher

## File structure 

- `hw1.cpp`: Contains all the code for the analysis 
- `cipher.txt`: Contains the ciphertext that needs to be converted into plaintext
- `english_quadgrams`: Contains the quadgrams statistics obtained by [1]

## Running the code 

Step 1: Compile the code: 
```bash 
g++ -std=c++17 -O3 -march=native -flto -funroll-loops -DNDEBUG hw1.cpp -o solver
```
Step 2: execute the code:
``` bash 
./solver
```

## Performance Analysis
To test the performance on your machine run the following command: 
```bash 
#if not installed 
sudo apt-get update 
sudo apt install hyperfine 
hyperfine -w 2 -r 50 './solver'
```



## Information theory primer 
To crack a substitution cipher, we start by relying on the fact that written text is highly predictable. This is due to the fact that the *entropy* (a measure of randomness) in natural language is typically low.

If we assume the source is **memoryless** (independent), entropy is computed as:

$$
H = - \sum_{x \in X} p(x) \log p(x)
$$

However, natural language cannot be accurately modeled as a memoryless source. Instead, we use the definition of **N-gram entropy** (or conditional entropy), as discussed in [2]:

$$
\begin{aligned}
F_N &= - \sum_{i,j} p(b_i, j) \log_2 p(j | b_i) \\
    &= - \sum_{i,j} p(b_i, j) \log_2 \frac{p(b_i, j)}{p(b_i)} \\
    &= - \sum_{i,j} p(b_i, j) \log_2 p(b_i, j) + \sum_{i,j} p(b_i, j) \log_2 p(b_i) \\
    &= H(\text{N-gram}) - H(\text{(N-1)-gram})
\end{aligned}
$$

Where:
- $b_i$ is a block of $N-1$ letters (an $(N-1)$-gram).
- $j$ is an arbitrary letter following $b_i$.
- $p(b_i, j)$ is the probability of the N-gram sequence $(b_i, j)$.

This demonstrates that N-gram entropy can be computed as the difference between the entropy of the N-grams and the entropy of the $(N-1)$-grams.

Now we can consider the entropy of written english as: 

$H = \lim_{N \to \infty}F_N$

This means that by taking a "big enough" N-gram entropy we can estimate the entropy of written english. 

### Using Quadgrams to identify english 

We rely on **Quadgrams** (N=4) rather than simple single-character frequencies (Unigrams) because natural language is not a memoryless source. 

If we only used single characters (Unigrams), we would be treating the text as a "bag of letters." A substitution cipher simply permutes these labels; it does not change the underlying set of frequencies. Therefore, a wrong decryption could easily match the *frequencies* of English (e.g., having many 'E's) while completely violating the *structure* of English (e.g., "E" followed by "X" and "Q").

By using Quadgrams, we capture the **conditional dependencies** of the language—the fact that the probability of a letter depends heavily on its predecessors (as shown by the entropy rate approximation $F_N$). 

Therefore, minimizing the **Cross-Entropy** of the Quadgram distribution ensures we are not just matching letter counts, but correctly reconstructing the sequential statistical structure of the English language.

### From Theory to Implementation: Log-Likelihood

While Information Theory suggests minimizing Cross-Entropy to break the cipher, our implementation maximizes the **Log-Likelihood** score. This approach is mathematically rigorous and computationally superior.

#### 1. Mathematical Equivalence (The Proof)
We can prove that maximizing Likelihood is identical to minimizing Cross-Entropy by examining their definitions.

Let $P$ be the empirical distribution of quadgrams in our decrypted text, and $Q$ be the standard distribution of English quadgrams (our model).

1.  **Cross-Entropy ($H$)** is defined formally as:
    $$H(P, Q) = - \sum_{x} P(x) \log Q(x)$$
    Where the summation is over all unique quadgrams $x$.

2.  **Log-Likelihood ($\mathcal{L}$)** of the text sequence $T$ (of length $N$) given model $Q$ is:
    $$\mathcal{L}(T) = \sum_{i=1}^{N} \log Q(q_i)$$

Since the empirical probability $P(x)$ of a quadgram is simply its count in the text divided by the text length ($Count(x)/N$), we can rewrite the Likelihood sum by grouping identical quadgrams:

$$
\begin{aligned}
\mathcal{L}(T) &= \sum_{x} Count(x) \log Q(x) \\
&= N \sum_{x} \frac{Count(x)}{N} \log Q(x) \\
&= N \sum_{x} P(x) \log Q(x)
\end{aligned}
$$

Substituting this back into the definition of Cross-Entropy (1):

$$H(P, Q) = - \frac{1}{N} \mathcal{L}(T)$$

This proves the direct relationship:
$$\text{maximize } \mathcal{L}(T) \iff \text{minimize } H(P, Q)$$

#### 2. Computational Speed
Using Log-Likelihood transforms the problem from expensive multiplications into fast additions. 

Calculating raw probability involves multiplying thousands of small numbers:
$$P(T) = Q(q_1) \cdot Q(q_2) \cdot ... \cdot Q(q_N)$$

By applying logarithms, we convert this into a sum:
$$\log P(T) = \log Q(q_1) + \log Q(q_2) + ... + \log Q(q_N)$$


## Generating the mapping 

In order to generate a proper mapping, we face a combinatorial explosion. A simple substitution cipher has a key space of $26!$ (approximately $4 \times 10^{26}$) possible permutations, making a brute-force search computationally impossible.

Instead, we treat the decryption process as an optimization problem where the objective is to minimize the Cross-Entropy cost function described above. A suitable heuristic for this optimization is the **Hill Climbing** algorithm.

### Stochastic Hill Climbing

Hill Climbing is an iterative algorithm that starts with an arbitrary solution and attempts to improve it by making small, incremental changes. In the context of a substitution cipher:

1.  **Initialization:** Generate a random initial key (mapping) $K_{parent}$.
2.  **Mutation:** Create a new key $K_{child}$ by swapping two random characters in $K_{parent}$ (e.g., swap 'A' and 'R').
3.  **Evaluation:** Decrypt the ciphertext using both keys and compute the Quadgram Cross-Entropy (fitness score) for both.
4.  **Selection:** * If $Score(K_{child}) < Score(K_{parent})$, the child is an improvement. We set $K_{parent} \leftarrow K_{child}$.
    * Otherwise, discard the child and try a different swap.
5.  **Termination:** Repeat steps 2-4 until the score stabilizes and no further improvements are found.



### Avoiding Local Optima with Random Restarts

A significant limitation of standard Hill Climbing is its tendency to get trapped in **local optima**. These are suboptimal mappings where any single swap of two letters results in a worse score, yet the text is not fully decrypted (e.g., half the words are correct, but the rest are gibberish).

To mitigate this, we employ a **Random Restart** strategy. Instead of running the algorithm once, we run the Hill Climbing process multiple times starting from completely different random initial keys. Since the landscape of the cost function is complex, different starting points will converge to different local optima. By selecting the result with the global minimum Cross-Entropy across all runs, we maximize the probability of finding the correct solution.

## References 
[1] J. Lyons, "Quadgram Statistics as a Fitness Measure," Practical Cryptography, 2012. [Online]. Available: http://practicalcryptography.com/cryptanalysis/text-characterisation/quadgrams/. [Accessed: Jan. 20, 2026].

[2] "Prediction and Entropy of Printed English" By C.E. Shannon
