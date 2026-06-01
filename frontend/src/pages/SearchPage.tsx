import { AlertCircle, SlidersHorizontal } from 'lucide-react';
import { useState } from 'react';

import { ResultCard } from '../components/ResultCard';
import { SearchBox } from '../components/SearchBox';
import { api } from '../services/api';
import type { SearchResult } from '../types/api';

export function SearchPage() {
  const [query, setQuery] = useState('');
  const [topK, setTopK] = useState(5);
  const [explain, setExplain] = useState(false);
  const [results, setResults] = useState<SearchResult[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [searched, setSearched] = useState(false);

  async function runSearch(nextQuery = query) {
    const trimmed = nextQuery.trim();
    if (!trimmed) {
      return;
    }

    setLoading(true);
    setError(null);
    setSearched(true);

    try {
      const response = await api.search(trimmed, topK, explain);
      setResults(response.results);
      setQuery(response.query);
    } catch (err) {
      setResults([]);
      setError(err instanceof Error ? err.message : 'Search failed.');
    } finally {
      setLoading(false);
    }
  }

  return (
    <div className="grid gap-6">
      <section className="rounded-lg border border-slate-200 bg-white p-4 shadow-sm sm:p-6">
        <div className="mb-5 flex flex-col gap-4 lg:flex-row lg:items-end lg:justify-between">
          <div className="max-w-3xl">
            <h1 className="text-2xl font-semibold tracking-normal text-slate-950 sm:text-3xl">
              Search
            </h1>
          </div>
          <div className="flex flex-wrap items-center gap-3 rounded-lg border border-slate-200 bg-slate-50 p-3">
            <SlidersHorizontal className="h-4 w-4 text-slate-500" aria-hidden="true" />
            <label className="flex items-center gap-2 text-sm font-medium text-slate-700">
              Top-k
              <input
                type="number"
                min={1}
                max={100}
                value={topK}
                onChange={(event) => setTopK(Number(event.target.value))}
                className="h-9 w-20 rounded-md border border-slate-300 bg-white px-2 text-sm outline-none focus:border-teal-500 focus:ring-2 focus:ring-teal-100"
              />
            </label>
            <label className="flex items-center gap-2 text-sm font-medium text-slate-700">
              <input
                type="checkbox"
                checked={explain}
                onChange={(event) => setExplain(event.target.checked)}
                className="h-4 w-4 rounded border-slate-300 text-teal-600 focus:ring-teal-500"
              />
              Explain
            </label>
          </div>
        </div>

        <SearchBox
          query={query}
          loading={loading}
          onQueryChange={setQuery}
          onSubmit={runSearch}
        />
      </section>

      {error ? (
        <div className="flex items-start gap-3 rounded-lg border border-rose-200 bg-rose-50 p-4 text-rose-800">
          <AlertCircle className="mt-0.5 h-5 w-5 shrink-0" aria-hidden="true" />
          <p className="text-sm font-medium">{error}</p>
        </div>
      ) : null}

      {loading ? (
        <div className="grid gap-3">
          {Array.from({ length: 3 }).map((_, index) => (
            <div key={index} className="h-32 animate-pulse rounded-lg bg-slate-200" />
          ))}
        </div>
      ) : null}

      {!loading && searched && results.length === 0 && !error ? (
        <div className="rounded-lg border border-slate-200 bg-white p-8 text-center text-slate-500">
          No results found.
        </div>
      ) : null}

      {!loading && results.length > 0 ? (
        <section className="grid gap-4">
          {results.map((result) => (
            <ResultCard key={`${result.doc_id}-${result.rank}`} result={result} explain={explain} />
          ))}
        </section>
      ) : null}
    </div>
  );
}
