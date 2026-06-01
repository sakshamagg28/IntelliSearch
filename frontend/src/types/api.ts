export interface RankingDetails {
  bm25_score: number;
  expansion_score: number;
  fuzzy_score: number;
  phrase_boost: number;
  matched_terms: string[];
  expansions: string[];
  fuzzy_matches: string[];
}

export interface SearchResult {
  rank: number;
  doc_id: number;
  document: string;
  score: number;
  snippet: string;
  ranking_details?: RankingDetails | null;
}

export interface SearchResponse {
  query: string;
  top_k: number;
  explain: boolean;
  results: SearchResult[];
}

export interface AutocompleteResponse {
  query: string;
  suggestions: string[];
}

export interface AnalyticsEntry {
  value: string;
  count: number;
}

export interface AnalyticsResponse {
  top_queries: AnalyticsEntry[];
  autocomplete_usage: AnalyticsEntry[];
}

export interface HealthResponse {
  status: string;
  engine: string;
  core_binary: string;
  dataset: string;
  core_binary_exists: boolean;
  dataset_exists: boolean;
}
