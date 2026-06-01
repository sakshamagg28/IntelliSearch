import { RefreshCw } from 'lucide-react';
import { useEffect, useState } from 'react';

import { AnalyticsTable } from '../components/AnalyticsTable';
import { api } from '../services/api';
import type { AnalyticsResponse } from '../types/api';

export function AnalyticsPage() {
  const [analytics, setAnalytics] = useState<AnalyticsResponse | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  async function loadAnalytics() {
    setLoading(true);
    setError(null);

    try {
      setAnalytics(await api.analytics(10));
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Analytics failed.');
    } finally {
      setLoading(false);
    }
  }

  useEffect(() => {
    void loadAnalytics();
  }, []);

  return (
    <div className="grid gap-6">
      <section className="flex flex-col gap-4 rounded-lg border border-slate-200 bg-white p-4 shadow-sm sm:flex-row sm:items-center sm:justify-between sm:p-6">
        <div>
          <h1 className="text-2xl font-semibold tracking-normal text-slate-950 sm:text-3xl">
            Analytics
          </h1>
        </div>
        <button
          type="button"
          onClick={() => void loadAnalytics()}
          disabled={loading}
          className="inline-flex min-h-10 items-center justify-center gap-2 rounded-md bg-slate-950 px-4 text-sm font-semibold text-white transition hover:bg-slate-800 disabled:cursor-not-allowed disabled:bg-slate-300"
        >
          <RefreshCw className={['h-4 w-4', loading ? 'animate-spin' : ''].join(' ')} />
          Refresh
        </button>
      </section>

      {error ? (
        <div className="rounded-lg border border-rose-200 bg-rose-50 p-4 text-sm font-medium text-rose-800">
          {error}
        </div>
      ) : null}

      <div className="grid gap-4 lg:grid-cols-2">
        <AnalyticsTable title="Top queries" entries={analytics?.top_queries ?? []} />
        <AnalyticsTable title="Autocomplete prefixes" entries={analytics?.autocomplete_usage ?? []} />
      </div>
    </div>
  );
}
