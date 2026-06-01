import { Loader2, Search, X } from 'lucide-react';
import { KeyboardEvent, useEffect, useMemo, useRef, useState } from 'react';

import { api } from '../services/api';

interface SearchBoxProps {
  query: string;
  loading: boolean;
  onQueryChange: (query: string) => void;
  onSubmit: (query: string) => void;
}

export function SearchBox({ query, loading, onQueryChange, onSubmit }: SearchBoxProps) {
  const [suggestions, setSuggestions] = useState<string[]>([]);
  const [activeIndex, setActiveIndex] = useState(-1);
  const [open, setOpen] = useState(false);
  const requestId = useRef(0);

  const trimmedQuery = useMemo(() => query.trim(), [query]);

  useEffect(() => {
    if (trimmedQuery.length < 2) {
      setSuggestions([]);
      setOpen(false);
      return;
    }

    const id = ++requestId.current;
    const timeout = window.setTimeout(() => {
      api
        .autocomplete(trimmedQuery, 8)
        .then((response) => {
          if (id === requestId.current) {
            setSuggestions(response.suggestions);
            setActiveIndex(response.suggestions.length > 0 ? 0 : -1);
            setOpen(response.suggestions.length > 0);
          }
        })
        .catch(() => {
          if (id === requestId.current) {
            setSuggestions([]);
            setOpen(false);
          }
        });
    }, 220);

    return () => window.clearTimeout(timeout);
  }, [trimmedQuery]);

  function submit(value = trimmedQuery) {
    if (!value) {
      return;
    }
    setOpen(false);
    onSubmit(value);
  }

  function handleKeyDown(event: KeyboardEvent<HTMLInputElement>) {
    if (event.key === 'ArrowDown' && suggestions.length > 0) {
      event.preventDefault();
      setOpen(true);
      setActiveIndex((index) => (index + 1) % suggestions.length);
    } else if (event.key === 'ArrowUp' && suggestions.length > 0) {
      event.preventDefault();
      setOpen(true);
      setActiveIndex((index) => (index <= 0 ? suggestions.length - 1 : index - 1));
    } else if (event.key === 'Enter') {
      event.preventDefault();
      const selected = open && activeIndex >= 0 ? suggestions[activeIndex] : trimmedQuery;
      onQueryChange(selected);
      submit(selected);
    } else if (event.key === 'Escape') {
      setOpen(false);
    }
  }

  return (
    <div className="relative">
      <div className="flex min-h-14 items-center gap-3 rounded-lg border border-slate-300 bg-white px-4 shadow-sm focus-within:border-teal-500 focus-within:ring-4 focus-within:ring-teal-100">
        <Search className="h-5 w-5 shrink-0 text-slate-400" aria-hidden="true" />
        <input
          value={query}
          onChange={(event) => onQueryChange(event.target.value)}
          onFocus={() => setOpen(suggestions.length > 0)}
          onKeyDown={handleKeyDown}
          placeholder="Search documents, topics, acronyms, or phrases"
          className="min-w-0 flex-1 bg-transparent text-base outline-none placeholder:text-slate-400"
        />
        {query && (
          <button
            type="button"
            onClick={() => {
              onQueryChange('');
              setOpen(false);
            }}
            className="grid h-8 w-8 place-items-center rounded-md text-slate-400 hover:bg-slate-100 hover:text-slate-700"
            aria-label="Clear search"
          >
            <X className="h-4 w-4" aria-hidden="true" />
          </button>
        )}
        <button
          type="button"
          onClick={() => submit()}
          disabled={loading || !trimmedQuery}
          className="inline-flex min-h-9 items-center gap-2 rounded-md bg-teal-600 px-4 text-sm font-semibold text-white transition hover:bg-teal-700 disabled:cursor-not-allowed disabled:bg-slate-300"
        >
          {loading ? <Loader2 className="h-4 w-4 animate-spin" aria-hidden="true" /> : null}
          Search
        </button>
      </div>

      {open && suggestions.length > 0 && (
        <div className="absolute z-20 mt-2 w-full overflow-hidden rounded-lg border border-slate-200 bg-white shadow-panel">
          {suggestions.map((suggestion, index) => (
            <button
              key={suggestion}
              type="button"
              onMouseDown={(event) => event.preventDefault()}
              onClick={() => {
                onQueryChange(suggestion);
                submit(suggestion);
              }}
              className={[
                'flex min-h-10 w-full items-center px-4 text-left text-sm',
                index === activeIndex
                  ? 'bg-teal-50 text-teal-800'
                  : 'text-slate-700 hover:bg-slate-50',
              ].join(' ')}
            >
              {suggestion}
            </button>
          ))}
        </div>
      )}
    </div>
  );
}
