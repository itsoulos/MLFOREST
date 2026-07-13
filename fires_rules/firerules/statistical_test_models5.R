## =====================================================================
## Statistical significance testing of machine learning model performance
## for forest fire danger classification (Attica, Eastern Macedonia &
## Thrace, Crete)
##
## INPUT:
##   stats_tbl.xlsx, read from INPUT_PATH below.
##   - 3 rows    = study regions (Attica, Eastern Macedonia & Thrace, Crete)
##   - 6 columns = ML models (MLP(BFGS), RBF, RF, FC2(RF), FC2(MLP), GENCLASS)
##   - cell values = average classification error (%) per region/model
##
## OUTPUT (written to the SAME folder as the input file):
##   1. "statistical_analysis_figure.png" - a plot for general readers,
##      showing mean classification error (+/- SD) per model, ranked,
##      with GENCLASS highlighted and significance markers vs. GENCLASS.
##   2. "statistical_analysis_table.xlsx" - a detailed results table for
##      researchers, with descriptive statistics, Friedman ranks, and
##      paired significance tests vs. GENCLASS.
##
## IMPORTANT LIMITATION:
##   Each model has only n = 3 observations (one per region), because
##   only region-level averages were available (not the 10 individual
##   cross-validation fold errors per region). This is a blocked design
##   (region = block, model = treatment) with limited statistical
##   power. Results should be interpreted as indicative rather than
##   definitive.
## =====================================================================

## ---- 0. Path configuration ----
## Change INPUT_PATH below to point to your stats_tbl.xlsx file.
## Both output files will be written to the SAME folder as this file.
INPUT_PATH <- "C:/Users/admin/Desktop/firerules/stats_tbl.xlsx"

OUTPUT_DIR    <- dirname(INPUT_PATH)
FIGURE_PATH   <- file.path(OUTPUT_DIR, "statistical_analysis_figure.png")
TABLE_PATH    <- file.path(OUTPUT_DIR, "statistical_analysis_table.xlsx")

## ---- 1. Setup ----
required_packages <- c("readxl", "dplyr", "tidyr", "ggplot2", "openxlsx")
to_install <- required_packages[!required_packages %in% installed.packages()[, "Package"]]
if (length(to_install) > 0) install.packages(to_install, repos = "https://cloud.r-project.org")

library(readxl)
library(dplyr)
library(tidyr)
library(ggplot2)
library(openxlsx)

## ---- 2. Load data ----
raw <- read_excel(INPUT_PATH, sheet = 1)
region_names <- c("Attica", "Eastern Macedonia & Thrace", "Crete")

data_wide <- raw
data_wide$Region <- region_names

data_long <- data_wide %>%
  pivot_longer(cols = -Region, names_to = "Model", values_to = "Error") %>%
  mutate(
    Region    = factor(Region, levels = region_names),
    Model = factor(Model, levels = colnames(raw))
  )

error_matrix <- as.matrix(data_wide[, colnames(raw)])
rownames(error_matrix) <- region_names

## ---- 3. Descriptive statistics per model ----
desc_stats <- data_long %>%
  group_by(Model) %>%
  summarise(
    Mean_Error = mean(Error),
    SD_Error   = sd(Error),
    Min_Error  = min(Error),
    Max_Error  = max(Error),
    .groups = "drop"
  )

## ---- 4. Friedman test (overall comparison) ----
friedman_result <- friedman.test(error_matrix)

## ---- 5. Average ranks + Nemenyi critical difference ----
k <- ncol(error_matrix)
n <- nrow(error_matrix)

rank_matrix <- t(apply(error_matrix, 1, rank))   # 1 = best (lowest error) within each region
avg_ranks   <- colMeans(rank_matrix)

q_alpha_table <- c("2"=1.960, "3"=2.343, "4"=2.569, "5"=2.728,
                   "6"=2.850, "7"=2.949, "8"=3.031, "9"=3.102, "10"=3.164)
q_alpha <- as.numeric(q_alpha_table[as.character(k)])
CD <- q_alpha * sqrt(k * (k + 1) / (6 * n))

## ---- 6. Paired comparisons: GENCLASS vs. each other model ----
genclass_errors  <- data_wide$GENCLASS
other_models <- setdiff(colnames(raw), "GENCLASS")

pairwise_tests <- lapply(other_models, function(tech) {
  other_errors <- data_wide[[tech]]
  wilcox_p <- tryCatch(
    wilcox.test(genclass_errors, other_errors, paired = TRUE, exact = FALSE)$p.value,
    error = function(e) NA)
  ttest_p <- tryCatch(
    t.test(genclass_errors, other_errors, paired = TRUE)$p.value,
    error = function(e) NA)
  data.frame(
    Model  = tech,
    Mean_Diff_vs_GENCLASS = mean(genclass_errors) - mean(other_errors),
    Wilcoxon_p = wilcox_p,
    Paired_t_p = ttest_p
  )
})
pairwise_tests <- do.call(rbind, pairwise_tests)

## ---- 7. Assemble the full results table (for researchers) ----
results_table <- desc_stats %>%
  mutate(Model = as.character(Model)) %>%
  left_join(
    data.frame(Model = names(avg_ranks), Avg_Rank = avg_ranks),
    by = "Model"
  ) %>%
  left_join(
    pairwise_tests %>% mutate(Model = as.character(Model)),
    by = "Model"
  ) %>%
  mutate(
    Mean_Diff_vs_GENCLASS = ifelse(Model == "GENCLASS", 0, Mean_Diff_vs_GENCLASS),
    Significance = case_when(
      Model == "GENCLASS"      ~ "Reference",
      is.na(Paired_t_p)             ~ NA_character_,
      Paired_t_p < 0.001            ~ "***",
      Paired_t_p < 0.01             ~ "**",
      Paired_t_p < 0.05             ~ "*",
      TRUE                          ~ "n.s."
    )
  ) %>%
  arrange(Mean_Error) %>%
  mutate(
    Mean_Error = round(Mean_Error, 2),
    SD_Error   = round(SD_Error, 2),
    Min_Error  = round(Min_Error, 2),
    Max_Error  = round(Max_Error, 2),
    Avg_Rank   = round(Avg_Rank, 2),
    Mean_Diff_vs_GENCLASS = round(Mean_Diff_vs_GENCLASS, 2),
    Wilcoxon_p = round(Wilcoxon_p, 4),
    Paired_t_p = round(Paired_t_p, 4)
  ) %>%
  select(Model, Mean_Error, SD_Error, Min_Error, Max_Error, Avg_Rank,
         Mean_Diff_vs_GENCLASS, Wilcoxon_p, Paired_t_p, Significance)

cat("\n================ RESULTS TABLE (for researchers) ================\n")
print(results_table, row.names = FALSE)

cat("\nFriedman test (overall comparison across all 6 models):\n")
cat("  chi-squared =", round(friedman_result$statistic, 3),
    ", df =", friedman_result$parameter,
    ", p-value =", round(friedman_result$p.value, 4), "\n")
cat("Nemenyi critical difference (alpha = 0.05):", round(CD, 3), "\n")
cat("\nSignificance codes: *** p<0.001  ** p<0.01  * p<0.05  n.s. = not significant\n")
cat("(based on paired t-test vs. GENCLASS across the", n, "study regions)\n")

## ---- 8. Save the results table as a formatted .xlsx workbook ----
wb <- createWorkbook()
addWorksheet(wb, "Statistical_Analysis")

writeData(wb, "Statistical_Analysis", results_table, startRow = 1, headerStyle = createStyle(
  textDecoration = "bold", fgFill = "#D9D9D9", border = "TopBottom"
))

## Add a short notes block below the table
notes_row <- nrow(results_table) + 3
writeData(wb, "Statistical_Analysis",
          x = data.frame(Notes = c(
            paste0("Friedman test: chi-squared = ", round(friedman_result$statistic, 3),
                   ", df = ", friedman_result$parameter,
                   ", p-value = ", round(friedman_result$p.value, 4)),
            paste0("Nemenyi critical difference (alpha = 0.05): ", round(CD, 3)),
            "Significance vs. GENCLASS (paired t-test): *** p<0.001, ** p<0.01, * p<0.05, n.s. = not significant",
            paste0("Note: n = ", n, " regions per model; results are indicative given the small sample size.")
          )),
          startRow = notes_row, colNames = FALSE)

setColWidths(wb, "Statistical_Analysis", cols = 1:10, widths = "auto")
saveWorkbook(wb, TABLE_PATH, overwrite = TRUE)
cat("\nSaved table to:", TABLE_PATH, "\n")

## ---- 9. Build the figure (for general readers) ----
plot_data <- desc_stats %>%
  mutate(Model = as.character(Model)) %>%
  left_join(
    results_table %>% select(Model, Significance),
    by = "Model"
  ) %>%
  mutate(
    Model = factor(Model, levels = Model[order(Mean_Error)]),
    Label = case_when(
      Significance == "Reference" ~ "best",
      Significance == "***"       ~ "***",
      Significance == "**"        ~ "**",
      Significance == "*"         ~ "*",
      TRUE                        ~ "n.s."
    ),
    Highlight = ifelse(Model == "GENCLASS", "GENCLASS", "Other models")
  )

fig <- ggplot(plot_data, aes(x = Model, y = Mean_Error, fill = Highlight)) +
  geom_col(width = 0.65, colour = "black", linewidth = 0.3) +
  geom_errorbar(aes(ymin = Mean_Error - SD_Error, ymax = Mean_Error + SD_Error),
                width = 0.2, linewidth = 0.5) +
  geom_text(aes(label = Label, y = Mean_Error + SD_Error + 0.6),
            size = 4.2, fontface = "bold") +
  geom_text(aes(label = paste0(round(Mean_Error, 2), "%"), y = Mean_Error + SD_Error + 1.9),
            size = 3.6) +
  scale_fill_manual(values = c("GENCLASS" = "#2E7D32", "Other models" = "#90A4AE")) +
  labs(
    title = "",
    subtitle = paste0("Friedman test: chi-squared = ", round(friedman_result$statistic, 2),
                       ", p = ", round(friedman_result$p.value, 3),
                       "  |  Significance markers show paired t-test vs. GENCLASS"),
    x = NULL,
    y = "Mean classification error [%] \u00b1 SD",
    fill = NULL
  ) +
  theme_minimal(base_size = 12) +
  theme(
    plot.title = element_text(face = "bold", size = 13),
    plot.subtitle = element_text(size = 9.5, colour = "grey30"),
    legend.position = "top",
    panel.grid.minor = element_blank(),
    axis.text.x = element_text(size = 10)
  ) +
  ylim(0, max(plot_data$Mean_Error + plot_data$SD_Error) + 3.5)

ggsave(FIGURE_PATH, fig, width = 8, height = 5.5, dpi = 300)
cat("Saved figure to:", FIGURE_PATH, "\n")

cat("\n================ END OF ANALYSIS ================\n")
