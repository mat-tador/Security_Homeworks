# Cracking a Substitution Cipher


## Information theory primer 
To crack a substitution cipher, we start by relying on the fact that written text is highly predictable. This is due to the fact that the *entropy* (a measure of randomness) in natural language is typically low.

If we assume the source is **memoryless** (independent), entropy is computed as:

$$
H = - \sum_{x \in X} p(x) \log p(x)
$$

However, natural language cannot be accurately modeled as a memoryless source. Instead, we use the definition of **N-gram entropy** (or conditional entropy), as discussed in [1]:

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
[1] "Prediction and Entropy of Printed English" By C.E. Shannon
