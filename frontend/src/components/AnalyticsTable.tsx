import type { AnalyticsEntry } from '../types/api';

interface AnalyticsTableProps {
  title: string;
  entries: AnalyticsEntry[];
}

export function AnalyticsTable({ title, entries }: AnalyticsTableProps) {
  const max = Math.max(...entries.map((entry) => entry.count), 1);

  return (
    <section className="rounded-lg border border-slate-200 bg-white p-5 shadow-sm">
      <div className="mb-4 flex items-center justify-between">
        <h2 className="text-base font-semibold text-slate-950">{title}</h2>
        <span className="rounded-md bg-slate-100 px-2 py-1 text-xs font-medium text-slate-500">
          {entries.length}
        </span>
      </div>

      {entries.length === 0 ? (
        <div className="rounded-lg border border-dashed border-slate-200 p-6 text-center text-sm text-slate-500">
          No data yet.
        </div>
      ) : (
        <div className="grid gap-3">
          {entries.map((entry) => (
            <div key={entry.value} className="grid gap-1">
              <div className="flex items-center justify-between gap-3 text-sm">
                <span className="min-w-0 truncate font-medium text-slate-700">{entry.value}</span>
                <span className="font-mono text-slate-500">{entry.count}</span>
              </div>
              <div className="h-2 rounded-full bg-slate-100">
                <div
                  className="h-2 rounded-full bg-indigo-500"
                  style={{ width: `${Math.max(6, (entry.count / max) * 100)}%` }}
                />
              </div>
            </div>
          ))}
        </div>
      )}
    </section>
  );
}
